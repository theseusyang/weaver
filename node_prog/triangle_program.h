#ifndef __TRIANGLE_PROG__
#define __TRIANGLE_PROG__
#include <vector>

#include "common/weaver_constants.h"
#include "db/element/node.h"
#include "db/element/remote_node.h"
#include "common/message.h"
#include "common/vclock.h"
#include "common/event_order.h"

namespace node_prog
{
    class triangle_params : public virtual Packable 
    {
        public:
            //uint64_t triangles_found;
            uint64_t responses_left;
            uint64_t num_edges;
            bool returning;
            std::vector<uint64_t> neighbors;
            db::element::remote_node super_node;
            db::element::remote_node vts_node;

        public:
            triangle_params()
                : returning(false)
                , reachable(false)
                , hops(0)
                , max_hops(0)
            {
            }
            
            virtual ~triangle_params() { }

            virtual uint64_t size() const 
            {
                uint64_t toRet = message::size(returning)
                    + message::size(prev_node)
                    + message::size(dest) 
                    //+ message::size(edge_props)
                    + message::size(hops)
                    + message::size(max_hops)
                    + message::size(reachable)
                    + message::size(path);
                return toRet;
            }

            virtual void pack(e::buffer::packer &packer) const 
            {
                message::pack_buffer(packer, returning);
                message::pack_buffer(packer, prev_node);
                message::pack_buffer(packer, dest);
                //message::pack_buffer(packer, edge_props);
                message::pack_buffer(packer, hops);
                message::pack_buffer(packer, max_hops);
                message::pack_buffer(packer, reachable);
                message::pack_buffer(packer, path);
            }

            virtual void unpack(e::unpacker &unpacker)
            {
                message::unpack_buffer(unpacker, returning);
                message::unpack_buffer(unpacker, prev_node);
                message::unpack_buffer(unpacker, dest);
                //message::unpack_buffer(unpacker, edge_props);
                message::unpack_buffer(unpacker, hops);
                message::unpack_buffer(unpacker, max_hops);
                message::unpack_buffer(unpacker, reachable);
                message::unpack_buffer(unpacker, path);
            }
    };

    struct triangle_node_state : Packable_Deletable
    {
        //bool visited;
        uint64_t already_visited_hops; // number of hops for a traversal that has alraedy visited this node
        db::element::remote_node prev_node; // previous node
        uint32_t out_count; // number of requests propagated
        bool reachable;
        uint64_t hops;

        triangle_node_state()
            : already_visited_hops(0)
            , out_count(0)
            , reachable(false)
            , hops(MAX_UINT64)
        { }

        virtual ~triangle_node_state() { }

        virtual uint64_t size() const 
        {
            uint64_t toRet = message::size(already_visited_hops)
                + message::size(prev_node)
                + message::size(out_count)
                + message::size(reachable)
                + message::size(hops);
            return toRet;
        }

        virtual void pack(e::buffer::packer& packer) const 
        {
            message::pack_buffer(packer, already_visited_hops);
            message::pack_buffer(packer, prev_node);
            message::pack_buffer(packer, out_count);
            message::pack_buffer(packer, reachable);
            message::pack_buffer(packer, hops);
        }

        virtual void unpack(e::unpacker& unpacker)
        {
            message::unpack_buffer(unpacker, already_visited_hops);
            message::unpack_buffer(unpacker, prev_node);
            message::unpack_buffer(unpacker, out_count);
            message::unpack_buffer(unpacker, reachable);
            message::unpack_buffer(unpacker, hops);
        }
    };

    inline int get_num_edges(db::element::node &n, vc::vclock &req_vclock) {
        int toRet = 0;
        for (auto &iter: n.out_edges) {
            e = iter.second;
            // check edge created and deleted in acceptable timeframe
            int64_t cmp_1 = order::compare_two_vts(e->get_creat_time(), req_vclock);
            assert(cmp_1 != 2);
            bool traverse_edge = (cmp_1 == 0);
            if (traverse_edge) {
                int64_t cmp_2 = order::compare_two_vts(e->get_del_time(), req_vclock);
                assert(cmp_2 != 2);
                traverse_edge = (cmp_2 == 1);
            }
            if (traverse_edge){
                toRet++;
            }
        }
        return toRet;
    }

    std::vector<std::pair<db::element::remote_node, triangle_params>> 
    triangle_node_program(uint64_t, // TODO used to be req_id, now replaced by vclock
            db::element::node &n,
            db::element::remote_node &rn,
            triangle_params &params,
            std::function<triangle_node_state&()> state_getter,
            vc::vclock &req_vclock)
    {
        DEBUG << "inside node prog!\n";
        std::vector<std::pair<db::element::remote_node, triangle_params>> next;
        if (rn.handle == super_node.handle) {
            triangle_node_state &state = state_getter();

            if (state.responses_left == 0) { // this only happens when state is not yet initialized
                state.responses_left = params.responses_left;
                state.total = 0;
            }
            if (!returning) { // this is the prog to count for the super node, happens once
                state.total += get_num_edges(n, req_vclock);

            } else {
                state.total += params.num_edges;
            }
            // at end, send total for this shard to coordinator
            if (--state.responses_left == 0) {
                params.num_edges = state.total;
                next.emplace_back(std::make_pair(params.vts_node, params));
            }
        } else {  // not at super node
            params.num_edges = get_num_edges(n, req_vclock);
            params.returning = true;
            // send to canonical node
            next.emplace_back(std::make_pair(super_node, params));
        }
        return next;
    }

    /*
    std::vector<std::pair<db::element::remote_node, reach_params>> 
    reach_node_deleted_program(uint64_t req_id,
                db::element::node &n, // node who asked to go to deleted node
                uint64_t deleted_handle, // handle of node that didn't exist
            reach_params &params_given, // params we had sent to deleted node
            std::function<reach_node_state&()> state_getter)
    {
        UNUSED(req_id);
        UNUSED(n);
        UNUSED(deleted_handle);
        UNUSED(params_given);
        UNUSED(state_getter);
        return std::vector<std::pair<db::element::remote_node, reach_params>>(); 
    }
    */
}

#endif //__TRIANGLE_PROG__
