/*
 * ================================================================
 *    Description:  Inter-server message packing and unpacking
 *
 *        Created:  11/07/2012 01:40:52 PM
 *
 *         Author:  Ayush Dubey, dubey@cs.cornell.edu
 *
 * Copyright (C) 2013, Cornell University, see the LICENSE file
 *                     for licensing agreement
 * ===============================================================
 */

#ifndef __MESSAGE__
#define __MESSAGE__

#include <memory>
#include <string.h>
#include <e/buffer.h>
#include <po6/net/location.h>
#include <busybee_constants.h>

#include "common/weaver_constants.h"
#include "common/property.h"
#include "common/meta_element.h"
#include "common/vclock.h"
#include "unordered_set"
#include "unordered_map"
#include "db/element/node.h"
#include "db/element/edge.h"
#include "db/element/remote_node.h"
#include "node_prog/node_prog_type.h" // used for packing Packable objects
#include "db/request_objects.h"

namespace message
{
    enum msg_type
    {
        CLIENT_NODE_CREATE_REQ = 0,
        CLIENT_EDGE_CREATE_REQ,
        CLIENT_NODE_DELETE_REQ,
        CLIENT_EDGE_DELETE_REQ,
        CLIENT_ADD_EDGE_PROP,
        CLIENT_DEL_EDGE_PROP,
        CLIENT_CLUSTERING_REQ,
        CLIENT_REACHABLE_REQ,
        CLIENT_REPLY,
        CLIENT_DIJKSTRA_REQ,
        CLIENT_DIJKSTRA_REPLY,
        CLIENT_CLUSTERING_REPLY,
        NODE_REFRESH_REQ,
        NODE_REFRESH_REPLY,
        NODE_CREATE_REQ,
        EDGE_CREATE_REQ,
        TRANSIT_EDGE_CREATE_REQ,
        REVERSE_EDGE_CREATE,
        TRANSIT_REVERSE_EDGE_CREATE,
        NODE_CREATE_ACK,
        EDGE_CREATE_ACK,
        TRANSIT_EDGE_CREATE_ACK,
        NODE_DELETE_REQ,
        TRANSIT_NODE_DELETE_REQ,
        EDGE_DELETE_REQ,
        TRANSIT_EDGE_DELETE_REQ,
        PERMANENT_DELETE_EDGE,
        PERMANENT_DELETE_EDGE_ACK,
        NODE_DELETE_ACK,
        EDGE_DELETE_ACK,
        EDGE_ADD_PROP,
        TRANSIT_EDGE_ADD_PROP,
        EDGE_DELETE_PROP,
        TRANSIT_EDGE_DELETE_PROP,
        EDGE_DELETE_PROP_ACK,
        CACHE_UPDATE,
        CACHE_UPDATE_ACK,
        MIGRATE_NODE_STEP1,
        MIGRATE_NODE_STEP4,
        MIGRATE_NODE_STEP6,
        COORD_NODE_MIGRATE,
        COORD_NODE_MIGRATE_ACK,
        MIGRATED_NBR_UPDATE,

        NODE_PROG,
        CLIENT_NODE_PROG_REQ,
        CLIENT_NODE_PROG_REPLY,

        ERROR
    };

    enum edge_direction
    {
        FIRST_TO_SECOND = 0,
        SECOND_TO_FIRST = 1
    };

    class message
    {
        public:
            message();
            message(enum msg_type t);
            message(message &copy);

        public:
            enum msg_type type;
            std::auto_ptr<e::buffer> buf;

        public:
            void change_type(enum msg_type t);
    };

    template <typename T1, typename T2> inline size_t size(const std::unordered_map<T1, T2>& t);
    template <typename T> inline size_t size(const std::unordered_set<T>& t);
    template <typename T> inline size_t size(const std::vector<T>& t);
    template <typename T1, typename T2> inline size_t size(const std::pair<T1, T2>& t);
    template <typename T1, typename T2, typename T3> inline size_t size(const std::tuple<T1, T2, T3>& t);

    template <typename T1, typename T2> inline void pack_buffer(e::buffer::packer& packer, const std::unordered_map<T1, T2>& t);
    template <typename T> inline void pack_buffer(e::buffer::packer& packer, const std::unordered_set<T>& t);
    template <typename T> inline void pack_buffer(e::buffer::packer& packer, const std::vector<T>& t);
    template <typename T1, typename T2> inline void pack_buffer(e::buffer::packer &packer, const std::pair<T1, T2>& t);
    template <typename T1, typename T2, typename T3> inline void pack_buffer(e::buffer::packer &packer, const std::tuple<T1, T2, T3>& t);

    template <typename T1, typename T2> inline void unpack_buffer(e::unpacker& unpacker, std::unordered_map<T1, T2>& t);
    template <typename T> inline void unpack_buffer(e::unpacker& unpacker, std::unordered_set<T>& t);
    template <typename T> inline void unpack_buffer(e::unpacker& unpacker, std::vector<T>& t);
    template <typename T1, typename T2> inline void unpack_buffer(e::unpacker& unpacker, std::pair<T1, T2>& t);
    template <typename T1, typename T2, typename T3> inline void unpack_buffer(e::unpacker& unpacker, std::tuple<T1, T2, T3>& t);

    inline
    message :: message()
        : type(ERROR)
    {
    }

    inline
    message :: message(enum msg_type t)
        : type(t)
    {
    }

    inline 
    message :: message(message &copy)
        : type(copy.type)
    {
        buf = copy.buf;
    }

    inline void
    message :: change_type(enum msg_type t)
    {
        type = t;
    }

// size templates
    inline size_t size(const node_prog::prog_type &t)
    {
        return sizeof(uint32_t);
    }
    inline size_t size(const node_prog::Packable &t)
    {
        return t.size();
    }
    inline size_t size(const bool &t)
    {
        return sizeof(uint16_t);
    }
    inline size_t size(const uint16_t &t)
    {
        return sizeof(uint16_t);
    }
    inline size_t size(const uint32_t &t)
    {
        return sizeof(uint32_t);
    }
    inline size_t size(const uint64_t &t)
    {
        return sizeof(uint64_t);
    }
    inline size_t size(const int &t)
    {
        return sizeof(int);
    }
    inline size_t size(const double &t)
    {
        return sizeof(uint64_t);
    }
    inline size_t size(const common::property &t)
    {
        return sizeof(uint32_t)+sizeof(size_t)+2*sizeof(uint64_t);
    }
    inline size_t size(const db::element::remote_node &t)
    {
        return size(t.loc) + size(t.handle);
    }
    inline size_t size(const db::element::edge* const &t)
    {
        size_t sz = 2*sizeof(uint64_t) + // time stamps
            size(*t->get_props()) + // properties
            size(t->nbr);
        return sz;
    }
    inline size_t size(const db::element::node &t)
    {
        size_t sz = 2*sizeof(uint64_t); // time stamps
        sz += size(*t.get_props());  // properties
        sz += size(t.out_edges);
        sz += size(t.in_edges);
        sz += size(t.update_count);
        sz += size(t.msg_count);
        return sz;
    }
    template <typename T1, typename T2>
    inline size_t size(const std::pair<T1, T2> &t)
    {
        return size(t.first) + size(t.second);
    }

    template <typename T1, typename T2, typename T3>
    inline size_t size(const std::tuple<T1, T2, T3>& t){
        return size(std::get<0>(t)) + size(std::get<1>(t)) + size(std::get<2>(t));
    }

    template <typename T>
    inline size_t size(const std::unordered_set<T> &t)
    {
        // O(n) size operation can handle elements of differing sizes
        size_t total_size = 0;
        for(const T &elem : t) {
            total_size += size(elem);
        }
        return sizeof(size_t)+total_size;
    }

    template <typename T1, typename T2>
    inline size_t size(const std::unordered_map<T1, T2> &t)
    {
        size_t total_size = 0;
        // O(n) size operation can handle keys and values of differing sizes
        for (const std::pair<T1,T2> &pair : t) {
            total_size += size(pair.first) + size(pair.second);
        }
        return sizeof(size_t)+total_size;
    }

    template <typename T>
    inline size_t size(const std::vector<T> &t)
    {
        size_t tot_size = sizeof(size_t);
        for (const T &elem : t) {
            tot_size += size(elem);
        }
        return tot_size;
    }

    /*
    template <typename T>
    inline size_t size(const std::vector<T> &t)
    {
        // first size_t to record size of vector, assumes constant size elements
        if (t.size()>0) {
            return sizeof(size_t) + (t.size()*size(t[0]));
        }
        else {
            return sizeof(size_t);
        }
    }
    */

    template <typename T, typename... Args>
    inline size_t size(const T &t, const Args&... args)
    {
        return size(t) + size(args...);
    }

    // packing templates

    inline void pack_buffer(e::buffer::packer &packer, const node_prog::Packable &t)
    {
        //std::cout << "pack buffer called for Packable type!" << std::endl;
        t.pack(packer);
    }
    inline void pack_buffer(e::buffer::packer &packer, const node_prog::prog_type &t)
    {
        packer = packer << t;
    //    std::cout << "pack buffer packed prog_type " << t << " of size " << sizeof(db::prog_type) << std::endl;
    }

    inline void pack_buffer(e::buffer::packer &packer, const bool &t)
    {
        uint16_t to_pack = t ? 1 : 0;
        packer = packer << to_pack;
    }
    inline void 
    pack_buffer(e::buffer::packer &packer, const uint16_t &t)
    {
        packer = packer << t;
    }
    inline void 
    pack_buffer(e::buffer::packer &packer, const uint32_t &t)
    {
        packer = packer << t;
    }
    inline void 
    pack_buffer(e::buffer::packer &packer, const uint64_t &t)
    {
        packer = packer << t;
    }
    inline void 
    pack_buffer(e::buffer::packer &packer, const int &t)
    {
        packer = packer << t;
    }

    inline void 
    pack_buffer(e::buffer::packer &packer, const double &t)
    {
        uint64_t dbl;
        memcpy(&dbl, &t, sizeof(double)); //to avoid casting issues, probably could avoid copy
        packer = packer << dbl;
    }

    inline void 
    pack_buffer(e::buffer::packer &packer, const common::property &t)
    {
        packer = packer << t.key << t.value << t.creat_time << t.del_time;
    }

    inline void 
    pack_buffer(e::buffer::packer &packer, const db::element::remote_node& t)
    {
        packer = packer << t.loc << t.handle;
    }

    template <typename T1, typename T2>
    inline void 
    pack_buffer(e::buffer::packer &packer, const std::pair<T1, T2> &t)
    {
        // assumes constant size
        pack_buffer(packer, t.first);
        pack_buffer(packer, t.second);
    }

    template <typename T1, typename T2, typename T3>
    inline void pack_buffer(e::buffer::packer &packer, const std::tuple<T1, T2, T3>& t){
        pack_buffer(packer, std::get<0>(t));
        pack_buffer(packer, std::get<1>(t));
        pack_buffer(packer, std::get<2>(t));
    }

    inline void pack_buffer(e::buffer::packer &packer, const db::element::edge* const &t)
    {
        packer = packer << t->get_creat_time() << t->get_del_time();
        pack_buffer(packer, *t->get_props());
        pack_buffer(packer, t->nbr);
    }

    inline void
    pack_buffer(e::buffer::packer &packer, const db::element::node &t)
    {
        packer = packer << t.get_creat_time() << t.get_del_time();
        pack_buffer(packer, *t.get_props());
        pack_buffer(packer, t.out_edges);
        pack_buffer(packer, t.in_edges);
        pack_buffer(packer, t.update_count);
        pack_buffer(packer, t.msg_count);
    }

    template <typename T> 
    inline void 
    pack_buffer(e::buffer::packer &packer, const std::vector<T> &t)
    {
        // !assumes constant element size
        size_t num_elems = t.size();
        //std::cout << "pack buffer packed vector of size " << num_elems<< std::endl;
        packer = packer << num_elems;
        if (num_elems > 0){
            //size_t element_size = size(t[0]);
            for (const T &elem : t) {
                pack_buffer(packer, elem);
            }
        }
    }

    /* vector with elements of constant size
    template <typename T> 
    inline void 
    pack_buffer(e::buffer::packer &packer, const std::vector<T> &t)
    {
        // !assumes constant element size
        size_t num_elems = t.size();
        //std::cout << "pack buffer packed vector of size " << num_elems<< std::endl;
        packer = packer << num_elems;
        if (num_elems > 0){
            size_t element_size = size(t[0]);
            for (const T &elem : t) {
                pack_buffer(packer, elem);
                assert(element_size == size(elem));//slow
            }
        }
    }
    */


    template <typename T>
    inline void 
    pack_buffer(e::buffer::packer &packer, const std::unordered_set<T> &t)
    {
        size_t num_keys = t.size();
        packer = packer << num_keys;
        for (const T &elem : t) {
            pack_buffer(packer, elem);
        }
    }

    template <typename T1, typename T2>
    inline void 
    pack_buffer(e::buffer::packer &packer, const std::unordered_map<T1, T2> &t)
    {
        size_t num_keys = t.size();
        packer = packer << num_keys;
        for (const std::pair<T1, T2> &pair : t) {
            pack_buffer(packer, pair.first);
            pack_buffer(packer, pair.second);
        }
    }

    template <typename T, typename... Args>
    inline void 
    pack_buffer(e::buffer::packer &packer, const T &t, const Args&... args)
    {
        pack_buffer(packer, t);
        pack_buffer(packer, args...);
    }

    inline void
    prepare_message(message &m, enum msg_type given_type)
    {
        uint32_t index = BUSYBEE_HEADER_SIZE;
        m.type = given_type;
        m.buf.reset(e::buffer::create(BUSYBEE_HEADER_SIZE + sizeof(enum msg_type)));

        m.buf->pack_at(index) << given_type;
    }

    template <typename... Args>
    inline void
    prepare_message(message &m, const enum msg_type given_type, const Args&... args)
    {
        size_t bytes_to_pack = size(args...) + sizeof(enum msg_type);
        /*
        if (given_type == NODE_PROG || given_type == CLIENT_NODE_PROG_REQ){
            std::cout << "preparing message contents of size " << bytes_to_pack << std::endl;
        }
        */
        m.type = given_type;
        m.buf.reset(e::buffer::create(BUSYBEE_HEADER_SIZE + bytes_to_pack));
        e::buffer::packer packer = m.buf->pack_at(BUSYBEE_HEADER_SIZE); 

        packer = packer << given_type;
        pack_buffer(packer, args...);
    }

    // unpacking templates
    inline void
    unpack_buffer(e::unpacker &unpacker, node_prog::Packable &t)
    {
        //std::cout << "unpack buffer called for Packable type!" << std::endl;
        t.unpack(unpacker);
    }
    inline void
    unpack_buffer(e::unpacker &unpacker, node_prog::prog_type &t){
        uint32_t _type;
        unpacker = unpacker >> _type;
        //std::cout << "unpack buffer got " << _type << std::endl;
        t = (enum node_prog::prog_type) _type;
    }
    inline void
    unpack_buffer(e::unpacker &unpacker, bool &t)
    {
        uint16_t temp;
        unpacker = unpacker >> temp;
        t = (temp != 0);
    }
    inline void
    unpack_buffer(e::unpacker &unpacker, uint16_t &t)
    {
        unpacker = unpacker >> t;
    }
    inline void
    unpack_buffer(e::unpacker &unpacker, uint32_t &t)
    {
        unpacker = unpacker >> t;
    }
    inline void 
    unpack_buffer(e::unpacker &unpacker, uint64_t &t)
    {
        unpacker = unpacker >> t;
    }
    inline void 
    unpack_buffer(e::unpacker &unpacker, int &t)
    {
        unpacker = unpacker >> t;
    }

    inline void 
    unpack_buffer(e::unpacker &unpacker, common::property &t)
    {
        unpacker = unpacker >> t.key >> t.value >> t.creat_time >> t.del_time;
    }

    inline void 
    unpack_buffer(e::unpacker &unpacker, double &t)
    {
        uint64_t dbl;
        unpacker = unpacker >> dbl;
        memcpy(&t, &dbl, sizeof(double)); //to avoid casting issues, probably could avoid copy
    }

    inline void 
    unpack_buffer(e::unpacker &unpacker, db::element::remote_node& t)
    {
        unpacker = unpacker >> t.loc >> t.handle;
    }

    template <typename T1, typename T2>
    inline void 
    unpack_buffer(e::unpacker &unpacker, std::pair<T1, T2> &t)
    {
        //assumes constant size
        unpack_buffer(unpacker, t.first);
        unpack_buffer(unpacker, t.second);
    }

    template <typename T1, typename T2, typename T3>
    inline void unpack_buffer(e::unpacker& unpacker, std::tuple<T1, T2, T3>& t){
        unpack_buffer(unpacker, std::get<0>(t));
        unpack_buffer(unpacker, std::get<1>(t));
        unpack_buffer(unpacker, std::get<2>(t));
    }

    inline void
    unpack_buffer(e::unpacker &unpacker, db::element::edge *&t)
    {
        uint64_t tc, td;
        std::vector<common::property> props;
        db::element::remote_node rn;
        size_t nbr_handle;
        int nbr_loc;
        unpacker = unpacker >> tc >> td;
        unpack_buffer(unpacker, props);
        unpack_buffer(unpacker, rn);
        t = new db::element::edge(tc, rn);
        t->update_del_time(td);
        t->set_properties(props);
    }

    inline void
    unpack_buffer(e::unpacker &unpacker, db::element::node &t)
    {
        uint64_t tc, td;
        std::vector<common::property> props;
        unpacker = unpacker >> tc >> td;
        unpack_buffer(unpacker, props);
        unpack_buffer(unpacker, t.out_edges);
        unpack_buffer(unpacker, t.in_edges);
        unpack_buffer(unpacker, t.update_count);
        unpack_buffer(unpacker, t.msg_count);
        t.update_creat_time(tc);
        t.update_del_time(td);
        t.set_properties(props);
    }

    template <typename T> 
    inline void 
    unpack_buffer(e::unpacker &unpacker, std::vector<T> &t)
    {
        assert(t.size() == 0);
        size_t elements_left;
        unpacker = unpacker >> elements_left;

        t.reserve(elements_left);

        while (elements_left > 0) {
            T to_add;
            unpack_buffer(unpacker, to_add);
            t.push_back(std::move(to_add));
            elements_left--;
        }
    }

    template <typename T>
    inline void 
    unpack_buffer(e::unpacker &unpacker, std::unordered_set<T> &t)
    {
        assert(t.size() == 0);
        size_t elements_left;
        unpacker = unpacker >> elements_left;

        t.rehash(elements_left*1.25); // set number of buckets to 1.25*elements it will contain

        while (elements_left > 0) {
            T to_add;
            unpack_buffer(unpacker, to_add);
            t.insert(std::move(to_add));
            elements_left--;
        }
    }

    template <typename T1, typename T2>
    inline void 
    unpack_buffer(e::unpacker &unpacker, std::unordered_map<T1, T2> &t)
    {
        assert(t.size() == 0);
        size_t elements_left;
        unpacker = unpacker >> elements_left;
        // set number of buckets to 1.25*elements it will contain
        // did not use reserve as max_load_factor is default 1
        t.rehash(elements_left*1.25); 

        while (elements_left > 0) {
            T1 key_to_add;
            T2 val_to_add;

            unpack_buffer(unpacker, key_to_add);

            unpack_buffer(unpacker, val_to_add);

            t[key_to_add] = std::move(val_to_add); // XXX change to insert later to reduce overhead
            elements_left--;
        }
    }

    template <typename T, typename... Args>
    inline void 
    unpack_buffer(e::unpacker &unpacker, T &t, Args&... args)
    {
        unpack_buffer(unpacker, t);
        unpack_buffer(unpacker, args...);
    }

    template <typename... Args>
    inline void
    unpack_message(const message &m, const enum msg_type expected_type, Args&... args)
    {
        uint32_t _type;
        e::unpacker unpacker = m.buf->unpack_from(BUSYBEE_HEADER_SIZE);
        unpacker = unpacker >> _type;
        assert((enum msg_type)_type == expected_type);

        unpack_buffer(unpacker, args...);
    }
} //namespace message

#endif //__MESSAGE__
