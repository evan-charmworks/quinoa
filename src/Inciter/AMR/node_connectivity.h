#ifndef AMR_node_connectivity_h
#define AMR_node_connectivity_h

#include <vector>

namespace AMR {

    /**
     * @brief This class stores the connectivity of the node. Simply what this
     * means is that it just a vector of node ids. The value of the vector is
     * the two nodes the new node joins, and the index is the node id
     */
    class node_connectivity_t {

        private:
            std::vector<node_pair_t> nodes;

        public:

            //node_connectivity_t() { } // default cons

            /**
             * @brief Method to add initial nodes to the store
             *
             * @param initial_size
             */
            void fill_initial_nodes(size_t initial_size)
            {
                for (size_t i = 0; i < initial_size; i++)
                {
                    // These can initially be 0 as initial nodes don't join any
                    // two others.. this could be updated to track
                    // intermediates, but this currently tracks "added" nodes
                    // nicely
                    add(0,0);
                }
            }

            /**
             * @brief Return size of node container -- the number of nodes
             *
             * @return Number of nodes
             */
            size_t size()
            {
                return nodes.size();
            }

            /**
             * @brief Getter into node storage
             *
             * @param id Id of the node to get
             *
             * @return The node_pair at the given id
             */
            node_pair_t get(size_t id)
            {
                return nodes.at(id);
            }

            /**
             * @brief function to calculate which node is opposite a
             * tet face
             *
             * @param face_list A list of faces on the tet
             * @param opposite_index The index for the face you want to know
             * the opposite node for
             *
             * @return An index (0-3) to tell you if A, B, C, or D
             * (respectively) is opposite the given face_list_t
             *
             * This function is tightly coupled (too coupled) to generate_face_lists
             *
             * generate_face_lists generates the faces {ABC, ABD, ACD, BCD} in
             * a fixed order. Opposite_index says the face from a face list we
             * care about. This function returns a number in the range {0,3} to
             * tell you  which node is missing from that face.
             *
             * I.e If opposite_index is 1, Node C is missing => 2.
             */
            static size_t face_list_opposite(face_list_t face_list, size_t opposite_index)
            {
                // TODO: make this actually inspect the face_list and be much
                // more robust...
                switch(opposite_index)
                {
                    case 0:  // ABC
                        return 3;
                    case 1:  // ABD
                        return 2;
                    case 2:  // ACD
                        return 1;
                    case 3:  // BCD
                        return 0;
                    default: // something went horribly wrong..
                        assert(0);
                        return 0;
                }
            }

            // TODO: Document this
            // Int becasue it's signed..
            int find(size_t A, size_t B)
            {
                size_t min = std::min(A,B);
                size_t max = std::max(A,B);

                for (size_t i = 0; i < size(); i++)
                {
                    // Did we find it?
                    node_pair_t n = get(i);
                    if (min == n[0] && max == n[1])
                    {
                        return i;
                    }
                }
                return -1;
            }

            // TODO: Document this
            size_t add(size_t A, size_t B)
            {
                if (A != 0 || B != 0)
                {
                    assert(A != B);
                    // TODO: Abstract to exists method. (Could have one for id,
                    // as well as one for val)

                    // check if already exists
                    int f = find(A,B);
                    if (f != -1) {
                        std::cout << "Connect already exits " << A << " " << B << std::endl;
                        return f;
                    }
                }

                nodes.push_back( {{std::min(A,B), std::max(A,B)}} );
                std::cout << " add " << size()-1 << " a = " <<  A << " b = " << B << std::endl;
                return size()-1;
            }

            /**
             * @brief Print connectivity a id: a-b
             */
            void print()
            {
                std::cout << "Connectivity" << std::endl;
                for (int i = 0; i < size(); i ++)
                {
                    std::cout << i << ": A " << get(i)[0] << " B " << get(i)[1] << std::endl;
                }
            }

    };
}

#endif // guard
