#ifndef QUINOA_MESH_ADAPTER_H
#define QUINOA_MESH_ADAPTER_H

#include "types.h"
#include "util.h"
#include "id_generator.h"

#include "marked_refinements_store.h"
#include "tet_store.h"

#include "node_connectivity.h"
#include "node_store.h"

#include "refinement.h"
#include "derefinement.h"

#include "Refinement_State.h"

namespace AMR {
    class mesh_adapter_t {

        public:

            // TODO: Set these in a better way
            const real_t derefinement_cut_off = 0.2;
            const real_t refinement_cut_off = 0.9;

            AMR::tet_store_t tet_store;
            AMR::node_connectivity_t node_connectivity;

            // for coord type stuff
            AMR::node_store_t node_store;

            AMR::refinement_t *refiner;

            void init_node_store(coord_array_t* m_x, coord_array_t* m_y, coord_array_t* m_z, size_t graph_size);
            void init_with_nodes(coord_array_t* m_x, coord_array_t* m_y, coord_array_t* m_z, size_t graph_size);
            void init(std::vector<size_t> m_tetinpoel, size_t num_nodes);

            void reconsile_node_posistions();

            void consume_tets( std::vector< std::size_t > m_tetinpoel );

            void evaluate_error_estimate();
            void uniform_refinement();

            int detect_compatibility(int num_locked_edges,
                    AMR::Refinement_Case refinement_case);

            void mark_refinement();
            void perform_refinement();

            void refinement_class_one(int num_to_refine, edge_list_t edge_list,
                    size_t tet_id);
            void refinement_class_two(edge_list_t edge_list, size_t tet_id);
            void refinement_class_three(size_t tet_id);

            void lock_tet_edges(size_t tet_id);
            void deactivate_tet_edges(size_t tet_id);
            bool check_valid_refinement_case(size_t child_id);

            std::vector< std::size_t >& get_active_inpoel();
            std::vector< std::size_t >& get_active_id_mapping();

            void print_tets();

    };
}

#endif //QUINOA_MESH_ADAPTER_H
