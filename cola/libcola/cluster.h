/*
 * vim: ts=4 sw=4 et tw=0 wm=0
 *
 * libcola - A library providing force-directed network layout using the 
 *           stress-majorization method subject to separation constraints.
 *
 * Copyright (C) 2006-2014  Monash University
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * See the file LICENSE.LGPL distributed with the library.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Author(s):  Tim Dwyer
 *             Michael Wybrow
*/

#ifndef COLA_CLUSTER_H
#define COLA_CLUSTER_H

#include <cstdio>

#include "libvpsc/rectangle.h"
#include "libvpsc/variable.h"

#include "libcola/compound_constraints.h"
#include "libcola/commondefs.h"

namespace cola {


/**
 * @brief  A cluster defines a hierarchical partitioning over the nodes
 *         which should be kept disjoint by the layout somehow.
 *
 * You should not use this directly.  This is an abstract base class.  
 * At the top level you should be using RootCluster, and then below that 
 * either RectangualarCLuster or ConvexCluster.
 */
class Cluster
{
    public:
        Cluster();

// To prevent C++ objects from being destroyed in garbage collected languages
// when the libraries are called from SWIG, we hide the declarations of the
// destructors and prevent generation of default destructors.
#ifndef SWIG
        virtual ~Cluster();
#endif
        virtual void computeBoundary(const vpsc::Rectangles& rs) = 0;
        virtual void computeBoundingRect(const vpsc::Rectangles& rs);

        /**
         * @brief Mark a rectangle as being a child of this cluster.
         *
         * @param[in] index  The index of the Rectangle in the rectangles vector.
         */
        virtual void addChildNode(unsigned index);
        /**
         * @brief Mark a cluster as being a sub-cluster of this cluster.
         *
         * @param[in] cluster  The Cluster to be marked as a sub-cluster.
         */
        void addChildCluster(Cluster *cluster);
        
        virtual double padding(void) const
        {
            return 0;
        }
        virtual double margin(void) const
        {
            return 0;
        }

        void setDesiredBounds(const vpsc::Rectangle bounds);
        void unsetDesiredBounds();
        void createVars(const vpsc::Dim dim, const vpsc::Rectangles& rs,
                vpsc::Variables& vars);
        virtual void printCreationCode(FILE *fp) const = 0;
        virtual int containsShape(unsigned index) const;
        virtual bool clusterIsFromFixedRectangle(void) const;
        virtual void outputToSVG(FILE *fp) const = 0;
        
        // Returns the total area covered by contents of this cluster
        // (not including space between nodes/clusters).
        double area(const vpsc::Rectangles& rs);
        
        // Sets bounds based on the finalPositions of vMin and vMax.
        //
        void updateBounds(const vpsc::Dim dim);
        
        virtual void computeVarRect(vpsc::Variables& vs, size_t dim);

        vpsc::Rectangle bounds;
        vpsc::Rectangle varRect;
        vpsc::Variable *vXMin, *vXMax, *vYMin, *vYMax;

        // This will be the id of the left/bottom boundary, 
        // and the right/top will be clusterVarId + 1.
        unsigned clusterVarId; 

        double varWeight;
        double internalEdgeWeightFactor;
        std::vector<unsigned> nodes;
        std::vector<Cluster*> clusters;
        std::valarray<double> hullX, hullY;
        
    private:
        bool desiredBoundsSet;
        vpsc::Rectangle desiredBounds;

        vpsc::Variable *vMin, *vMax;
};
typedef std::vector<Cluster*> Clusters;


/**
 * @brief Holds the cluster hierarchy specification for a diagram.
 *
 * This is not considered a cluster itself, but it records all the nodes in 
 * the diagram not contained within any clusters, as well as optionally a 
 * hierarchy of clusters.
 *
 * You can add clusters via addChildCluster() and nodes via addChildNode().
 *
 * You can specify just the shapes contained in clusters, but not the nodes 
 * at this top level---the library will add any remaining nodes not appearing 
 * in the cluster hierarchy as children of the root cluster.
 *
 * It is possible to add a node as the child of two parent clusters.  In this 
 * case, the clusters will overlap to contain this (and possibly other nodes).
 * The library will warn you if you do this unless you have called the method
 * setAllowsMultipleParents() to mark this intention.
 *
 * Be careful not to create cycles in the cluster hierarchy (i.e., to mark 
 * two clusters as children of each other.  The library does not check for 
 * this and strange things may occur.
 */
class RootCluster : public Cluster 
{
    public:
        RootCluster();
        void computeBoundary(const vpsc::Rectangles& rs);
        // There are just shapes at the top level, so
        // effectively no clusters in the diagram scene.
        bool flat(void) const
        {
            return clusters.empty();
        }
        virtual void printCreationCode(FILE *fp) const;
        virtual void outputToSVG(FILE *fp) const;

        //! Returns true if this cluster hierarchy allows multiple parents, 
        //! otherwise returns false.
        //!
        //! Defaults to false.  If this is false, the library will display
        //! warnings if you add a single node to multiple clusters.
        //!
        //! @sa setAllowsMultipleParents()
        bool allowsMultipleParents(void) const;

        //! Set whether the cluster hierarchy should allow multiple parents.
        //! 
        //! @param value New value for this setting.
        //!
        //! sa allowsMultipleParents()
        void setAllowsMultipleParents(const bool value);
   private:
        bool m_allows_multiple_parents;
};

/**
 * @brief  Defines a rectangular cluster, either variable-sized with floating 
 *         sides or a fixed size based on a particular rectangle.
 *
 * The chosen constructor decides the type and behaviour of the cluster.
 */
class RectangularCluster : public Cluster
{
    public:
        /**
         * @brief  A rectangular cluster of variable size that contains
         *         its children.
         */
        RectangularCluster();
        /**
         * @brief  A fixed size rectangular cluster based on a particular 
         *         rectangle.
         *
         * This rectangle might be constrained in the other ways like normal 
         * rectangles.
         *
         * @param[in]  rectIndex  The index of the rectangle that this cluster
         *                        represents.
         */
        RectangularCluster(unsigned rectIndex);

        /**
         * @brief  Sets the margin size for this cluster.
         *
         * This value represents the outer spacing that will be put between
         * the cluster boundary and other clusters (plus their margin) and 
         * rectangles at the same level when non-overlap constraints are 
         * enabled.
         *
         * @param[in]  margin  The size of the margin for this cluster.
         */
        void setMargin(double margin);
        /**
         * @brief  Returns the margin size for this cluster.
         *
         * @return  The margin size for the cluster.
         */
        double margin(void) const;
        /**
         * @brief  Sets the padding size for this cluster.
         *
         * This value represents the inner spacing that will be put between
         * the cluster boundary and other child clusters (plus their margin) 
         * and child rectangles.
         *
         * @param[in]  padding  The size of the padding for this cluster.
         */
        void setPadding(double padding);
        /**
         * @brief  Returns the padding size for this cluster.
         *
         * @return  The padding size for the cluster.
         */
        double padding(void) const;

#ifndef SWIG
        virtual ~RectangularCluster();
#endif
        void computeBoundary(const vpsc::Rectangles& rs);
        virtual int containsShape(unsigned index) const;
        virtual void printCreationCode(FILE *fp) const;
        virtual void outputToSVG(FILE *fp) const;
        virtual void computeBoundingRect(const vpsc::Rectangles& rs);
        virtual void addChildNode(unsigned index);
        inline vpsc::Rectangle *getMinEdgeRect(const vpsc::Dim dim)
        {
            if (minEdgeRect[dim])
            {
                delete minEdgeRect[dim];
            }
            minEdgeRect[dim] = new vpsc::Rectangle(bounds);

            // Set the Min and Max positions to be the min minus an offset.
            double edgePosition = minEdgeRect[dim]->getMinD(dim);
            minEdgeRect[dim]->setMinD(dim, edgePosition - m_margin);
            minEdgeRect[dim]->setMaxD(dim, edgePosition);

            return minEdgeRect[dim];
        }

        inline vpsc::Rectangle *getMaxEdgeRect(const vpsc::Dim dim)
        {
            if (maxEdgeRect[dim])
            {
                delete maxEdgeRect[dim];
            }
            maxEdgeRect[dim] = new vpsc::Rectangle(bounds);

            // Set the Min and Max positions to be the max plus an offset.
            double edgePosition = maxEdgeRect[dim]->getMaxD(dim);
            maxEdgeRect[dim]->setMaxD(dim, edgePosition + m_margin);
            maxEdgeRect[dim]->setMinD(dim, edgePosition);

            return maxEdgeRect[dim];
        }
        virtual bool clusterIsFromFixedRectangle(void) const;
        int rectangleIndex(void) const;
    
        // For fixed sized clusters based on a rectangle, this method 
        // generates the constraints that attach the cluster edges to the
        // centre position of the relevant rectangle.
        void generateFixedRectangleConstraints(
                cola::CompoundConstraints& idleConstraints,
                vpsc::Rectangles& rc, vpsc::Variables (&vars)[2]) const;
    
    private:
        vpsc::Rectangle *minEdgeRect[2];
        vpsc::Rectangle *maxEdgeRect[2];
        int m_rectangle_index;
        double m_margin;
        double m_padding;
};

/**
 * @brief  Defines a cluster that will be treated as a convex boundary around
 *         the child nodes and clusters.
 */
class ConvexCluster : public Cluster
{
    public:
        void computeBoundary(const vpsc::Rectangles& rs);
        virtual void printCreationCode(FILE *fp) const;
        virtual void outputToSVG(FILE *fp) const;

        std::valarray<unsigned> hullRIDs;
        std::valarray<unsigned char> hullCorners;
};


} // namespace cola
#endif // COLA_CLUSTER_H
