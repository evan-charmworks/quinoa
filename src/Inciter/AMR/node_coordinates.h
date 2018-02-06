#ifndef AMR_node_coordinates_h
#define AMR_node_coordinates_h

#include <cmath>

#include "Base/Exception.h"
#include "AMR_types.h"
#include "tet_store.h"

// TODO: make this have a base class to support multiple generator schemes
// using the policy design pattern
namespace AMR {

    class node_coordinates_t
    {

        private:
            coord_type m_x;
            coord_type m_y;
            coord_type m_z;

            // We really don't want people to pass this by value..
            // (Because we store refs in here, which are consts..)
            // NonCopyable & operator=(const NonCopyable&) = delete;
            node_coordinates_t(const node_coordinates_t& c) = delete;
            node_coordinates_t& operator=(const node_coordinates_t&) = delete;

        public:
            // TODO: This needs to set the member variables
            node_coordinates_t() { } // default cons

            // TODO: Remove this and use x.size()
            size_t m_graphsize;

            // Deep copy
            void set_x_array(coord_type& x_in) { m_x = x_in; }
            void set_y_array(coord_type& y_in) { m_y = y_in; }
            void set_z_array(coord_type& z_in) { m_z = z_in; }

            // Getters to grab inside of an entire array
            coord_type* get_x_array() { return &m_x; }
            coord_type* get_y_array() { return &m_y; }
            coord_type* get_z_array() { return &m_z; }

            /**
             * @brief Function to enable the tracking of node coordinateds. Must be
             * called after node_connectivity is inited (by mesh_adapter.init())
             *
             * @param m_x Array of initial x coords
             * @param m_y Array of initial y coords
             * @param m_z Array of initial z coords
             * @param graph_size Number of nodes
             * // TODO: grpah_size can be deduced from size of x/y/z?
             */
            void init(coord_type& x, coord_type& y, coord_type& z, size_t graph_size)
            {
                Assert( x.size() == y.size(), "Coord arrays have different sizes");
                Assert( x.size() == z.size(), "Coord arrays have different sizes");

                set_x_array(x);
                set_y_array(y);
                set_z_array(z);
                m_graphsize = graph_size;
            }

            /**
             * @brief Function to add x coordinate data
             *
             * @param xc Data to add
             */
            void add_x(real_t xc) { m_x.push_back(xc); }

            /**
             * @brief Function to add y coordinate data
             *
             * @param yc Data to add
             */
            void add_y(real_t yc) { m_y.push_back(yc); }

            /**
             * @brief Function to add z coordinate data
             *
             * @param zc data to add
             */
            void add_z(real_t zc) { m_z.push_back(zc); }

            real_t x(size_t id)
            {
                Assert( id < size(), "ID out of bounds");
                return m_x[id];
            }
            real_t y(size_t id)
            {
                Assert( id < size(), "ID out of bounds");
                return m_y[id];
            }
            real_t z(size_t id)
            {
                Assert( id < size(), "ID out of bounds");
                return m_z[id];
            }

            // Alias to above. Should probably settle on a single interface?
            real_t get_x(size_t id) { return x(id); }
            real_t get_y(size_t id) { return y(id); }
            real_t get_z(size_t id) { return z(id); }

            size_t size()
            {
                return m_x.size();
            }

            // TODO: Document this
            void print()
            {
                for (size_t i = 0; i < size(); i++)
                {
                    std::cout << "Node " << i << " has coords :" <<
                        x(i) << ", " <<
                        y(i) << ", " <<
                        z(i) << ", " <<
                        std::endl;
               }
            }


            /**
             * @brief Function to add a new node
             *
             * @param xc x val of node
             * @param yc y val of node
             * @param zc z val of node
             *
             * @return id of node added
             */
            size_t add(real_t xc, real_t yc, real_t zc) {

                // Need to: Add to {xc,yc,zc} Add any connectivity?

                // Check if the node already exists
                int already_exists = check_node_exists(xc,yc,zc);

                if (already_exists == -1) {
                    size_t return_node_id = add_coordinates(xc,yc,zc);
                    m_graphsize++; // TODO: how best to deal with this?
                    return return_node_id;
                }
                else {
                    return static_cast<size_t>(already_exists);
                }

            }

            // TODO: Document this
            size_t add_between(size_t A, size_t B)
            {
                coordinate_t c = find_mid_point(A, B);
                return add(c);
            }

            /**
             * @brief Function to add a new node
             *
             * @param coord_tuple The coordinate data to add for the node
             *
             * @return Id of node added
             */
            size_t add(coordinate_t coord_tuple)
            {
                return add( coord_tuple[0], coord_tuple[1], coord_tuple[2]);
            }

            /**
             * @brief Function to add a new point/coordinates
             *
             * @param xc x val
             * @param yc y val
             * @param zc z val
             *
             * @return id of coordinate added
             */
            size_t add_coordinates(real_t xc, real_t yc, real_t zc) {
                add_x(xc);
                add_y(yc);
                add_z(zc);
                return size()-1; // -1 because of the 0 index
            }

            /**
             * @brief Function to add a new node
             *
             * @param xc x val of node
             * @param yc y val of node
             * @param zc z val of node
             *
             * @return id of node added
             */
            size_t add_node(real_t xc, real_t yc, real_t zc) {

                // Need to: Add to {xc,y,z} Add any connectivity?

                // Check if the node already exists
                int already_exists = check_node_exists(xc,yc,zc);

                if (already_exists == -1) {
                    size_t return_node_id = add_coordinates(xc,yc,zc);
                    m_graphsize++; // TODO: how to deal with this?
                    return return_node_id;
                }
                else {
                    return static_cast<size_t>(already_exists);
                }

            }

            // TODO: Remove all calls to this as it's fairly expensive...
            // TODO: Find a more cost effective way to implement this
                // Most likely change data structure for a faster search
                // This is also going to be a potential problem in async parallel
            /**
             * @brief Helper function to check if a node already exists at
             * coords {x,y,z} to avoid it being duplicated
             *
             * @param x_in X coord to check
             * @param y_in Y coord to check
             * @param z_in Z coord to check
             *
             * @return The id of the node if it exists, -1 if it doesn't.
             */
            int check_node_exists(real_t x_in, real_t y_in, real_t z_in)
            {
                const real_t eps = 1e-7;
                for (size_t i = 0; i < size(); i++)
                {
                    if (
                        std::abs( x(i) - x_in) < eps &&
                        std::abs( y(i) - y_in) < eps &&
                        std::abs( z(i) - z_in) < eps
                    )
                    {
                        return static_cast<int>(i);
                    }
                }

                return -1;
            }

            /**
             * @brief function to find the mid point between two points (nodes)
             * based on ids
             *
             * @param id1 id of the first node/point
             * @param id2 id of the second node/point
             *
             * @return The mid point
             */
            coordinate_t find_mid_point(size_t id1, size_t id2)
            {
                coordinate_t edge_node_A = id_to_coordinate(id1);
                coordinate_t edge_node_B = id_to_coordinate(id2);
                return AMR::util::find_mid_point(edge_node_A, edge_node_B);
            }

            /**
             * @brief Function to gather the {x,y,z} coordinates of a node from
             * index id
             *
             * @param id Id of node to gather (direct index into m_x[])
             *
             * @return List (array) of coordinate data
             */
            coordinate_t id_to_coordinate(size_t id)
            {
                Assert( id < size(), "Invalid ID");

                // Note: extra braces are to appease Clangs warning generator.
                //   (It's probably OK to remove them....)
                coordinate_t c = { {x(id), y(id), z(id) } };
                return c;
            }

            void set_x(size_t id, real_t val) { m_x[id] = val; }
            void set_y(size_t id, real_t val) { m_y[id] = val; }
            void set_z(size_t id, real_t val) { m_z[id] = val; }

            void set(size_t id, real_t x, real_t y, real_t z)
            {
                Assert( id < size(), "Invalid ID");
                set_x(id, x);
                set_y(id, y);
                set_z(id, z);
            }

            coordinate_t get(size_t id)
            {
                Assert( id < size(), "Invalid ID");
                return id_to_coordinate(id);
            }

    }; // end class
}

#endif  // guard
