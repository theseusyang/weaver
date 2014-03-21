#! /usr/bin/env python
# 
# ===============================================================
#    Description:  Read-only multi-client benchmark which only
#                  reads 1 vertex per query. 
# 
#        Created:  2014-03-21 13:39:06
# 
#         Author:  Ayush Dubey, dubey@cs.cornell.edu
# 
# Copyright (C) 2013-2014, Cornell University, see the LICENSE
#                     file for licensing agreement
# ===============================================================
# 

import random
import sys
import time
import threading
sys.path.append('../../.libs')

import libclient as client

num_started = 0
num_finished = 0
cv = threading.Condition()
num_requests = 5000
num_nodes = 81306 # snap twitter-combined
# node handles are range(0, num_nodes)
num_vts = 1
num_clients = 8

def exec_reads(reqs, cl, exec_time, idx):
    global num_started
    global cv
    global num_clients
    with cv:
        while num_started < num_clients:
            cv.wait()
    rp = client.ReadNodePropsParams()
    start = time.time()
    cnt = 0
    for r in reqs:
        cnt += 1
        prog_args = [(r, rp)]
        response = cl.read_node_props(prog_args)
        if cnt % 1000 == 0:
            print 'done ' + str(cnt) + ' by client ' + str(idx)
    end = time.time()
    with cv:
        num_finished += 1
        cv.notify_all()
    exec_time[idx] = end - start

clients = []
for i in range(num_clients):
    clients.append(client.Client(client._CLIENT_ID + i, i % num_vts))

# randomly write node props
# with p = 0.50 nodes have 0 props
# with p = 0.25 nodes have 1 props
# with p = 0.25 nodes have 2 props
write_nodes = (num_nodes / 1000) * 1000
tx_id = 0
c = clients[0]
tx_sz = 1000
for n in range(write_nodes):
    if n % tx_sz == 0:
        tx_id = c.begin_tx()
    coin_toss = random.random()
    if coin_toss > 0.50:
        c.set_node_property(tx_id, n, 'color', 'blue')
    if coin_toss > 0.75:
        c.set_node_property(tx_id, n, 'type', 'photo')
    if n % tx_sz == (tx_sz-1):
        c.end_tx(tx_id)

reqs = []
for i in range(num_clients):
    cl_reqs = []
    for numr in range(num_requests):
        cl_reqs.append(random.randint(0, num_nodes-1))
    reqs.append(cl_reqs)

exec_time = [0] * num_clients
threads = []
print "starting requests"
for i in range(num_clients):
    thr = threading.Thread(target=exec_reads, args=(reqs[i], clients[i], exec_time, i))
    thr.start()
    threads.append(thr)
start_time = time.time()
with cv:
    num_started = num_clients
    cv.notify_all()
    while num_finished < num_clients:
        cv.wait()
end_time = time.time()
total_time = end_time-start_time
for thr in threads:
    thr.join()
print 'Total time = ' + str(total_time)
throughput = (num_requests * num_clients) / total_time
print 'Throughput = ' + str(throughput)