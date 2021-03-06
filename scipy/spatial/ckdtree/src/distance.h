struct Dist1D {
    static inline void 
    interval_interval(const ckdtree * tree, 
                        const Rectangle& rect1, const Rectangle& rect2,
                        const npy_intp k,
                        npy_float64 *min, npy_float64 *max)
    {
        /* Compute the minimum/maximum distance along dimension k between points in
         * two hyperrectangles.
         */
        *min = dmax(0, dmax(rect1.mins[k] - rect2.maxes[k],
                              rect2.mins[k] - rect1.maxes[k]));
        *max = dmax(rect1.maxes[k] - rect2.mins[k], 
                              rect2.maxes[k] - rect1.mins[k]);
    }

    static inline npy_float64
    point_point(const ckdtree * tree, 
               const npy_float64 *x, const npy_float64 *y,
                 const npy_intp k) {
        return dabs(x[k] - y[k]);
    }
};

template <typename Dist1D>
struct BaseMinkowskiDistP1 {

    static inline void 
    interval_interval_p(const ckdtree * tree, 
                        const Rectangle& rect1, const Rectangle& rect2,
                        const npy_intp k, const npy_float64 p,
                        npy_float64 *min, npy_float64 *max)
    {
        /* Compute the minimum/maximum distance along dimension k between points in
         * two hyperrectangles.
         */
        Dist1D::interval_interval(tree, rect1, rect2, k, min, max);
    }

    static inline void 
    rect_rect_p(const ckdtree * tree, 
                        const Rectangle& rect1, const Rectangle& rect2,
                        const npy_float64 p,
                        npy_float64 *min, npy_float64 *max)
    {
        *min = 0.;
        *max = 0.;
        for(npy_intp i=0; i<rect1.m; ++i) {
            npy_float64 min_, max_;

            Dist1D::interval_interval(tree, rect1, rect2, i, &min_, &max_);

            *min += min_;
            *max += max_;
        }
    }

    static inline npy_float64 
    distance_p(const ckdtree * tree, 
               const npy_float64 *x, const npy_float64 *y,
               const npy_float64 p, const npy_intp k,
               const npy_float64 upperbound)
    {    
        npy_intp i;
        npy_float64 r;
        r = 0;
        for (i=0; i<k; ++i) {
            r += Dist1D::point_point(tree, x, y, i);
            if (r>upperbound)
                return r;
        }
        return r;
    }
};

template <typename Dist1D>
struct BaseMinkowskiDistPp {
    /* 1-d pieces
     * These should only be used if p != infinity
     */

    static inline void 
    interval_interval_p(const ckdtree * tree, 
                        const Rectangle& rect1, const Rectangle& rect2,
                        const npy_intp k, const npy_float64 p,
                        npy_float64 *min, npy_float64 *max)
    {
        /* Compute the minimum/maximum distance along dimension k between points in
         * two hyperrectangles.
         */
        Dist1D::interval_interval(tree, rect1, rect2, k, min, max);
        *min = std::pow(*min, p);
        *max = std::pow(*max, p);
    }

    static inline void 
    rect_rect_p(const ckdtree * tree, 
                        const Rectangle& rect1, const Rectangle& rect2,
                        const npy_float64 p,
                        npy_float64 *min, npy_float64 *max)
    {
        *min = 0.;
        *max = 0.;
        for(npy_intp i=0; i<rect1.m; ++i) {
            npy_float64 min_, max_;

            Dist1D::interval_interval(tree, rect1, rect2, i, &min_, &max_);

            *min += std::pow(min_, p);
            *max += std::pow(max_, p);
        }
    }

    static inline npy_float64 
    distance_p(const ckdtree * tree, 
               const npy_float64 *x, const npy_float64 *y,
               const npy_float64 p, const npy_intp k,
               const npy_float64 upperbound)
    {    
       /*
        * Compute the distance between x and y
        *
        * Computes the Minkowski p-distance to the power p between two points.
        * If the distance**p is larger than upperbound, then any number larger
        * than upperbound may be returned (the calculation is truncated).
        */
        
        npy_intp i;
        npy_float64 r, r1;
        r = 0;
        for (i=0; i<k; ++i) {
            r1 = Dist1D::point_point(tree, x, y, i);
            r += std::pow(r1, p);
            if (r>upperbound)
                 return r;
        }
        return r;
    } 
};

template <typename Dist1D>
struct BaseMinkowskiDistPinf {
    static inline void 
    interval_interval_p(const ckdtree * tree,
                        const Rectangle& rect1, const Rectangle& rect2,
                        const npy_intp k, npy_float64 p,
                        npy_float64 *min, npy_float64 *max)
    {
        return rect_rect_p(tree, rect1, rect2, p, min, max);
    }

    static inline void 
    rect_rect_p(const ckdtree * tree, 
                        const Rectangle& rect1, const Rectangle& rect2,
                        const npy_float64 p,
                        npy_float64 *min, npy_float64 *max)
    {
        *min = 0.;
        *max = 0.;
        for(npy_intp i=0; i<rect1.m; ++i) {
            npy_float64 min_, max_;

            Dist1D::interval_interval(tree, rect1, rect2, i, &min_, &max_);

            *min = dmax(*min, min_);
            *max = dmax(*max, max_);
        }
    }

    static inline npy_float64 
    distance_p(const ckdtree * tree, 
               const npy_float64 *x, const npy_float64 *y,
               const npy_float64 p, const npy_intp k,
               const npy_float64 upperbound)
    {    
        npy_intp i;
        npy_float64 r;
        r = 0;
        for (i=0; i<k; ++i) {
            r = dmax(r,Dist1D::point_point(tree, x, y, i));
            if (r>upperbound)
                return r;
        }
        return r;
    }
};

template <typename Dist1D>
struct BaseMinkowskiDistP2 {
    static inline void 
    interval_interval_p(const ckdtree * tree,
                        const Rectangle& rect1, const Rectangle& rect2,
                        const npy_intp k, const npy_float64 p,
                        npy_float64 *min, npy_float64 *max)
    {
        /* Compute the minimum/maximum distance along dimension k between points in
         * two hyperrectangles.
         */
        Dist1D::interval_interval(tree, rect1, rect2, k, min, max);
        *min *= *min;
        *max *= *max;
    }

    static inline void 
    rect_rect_p(const ckdtree * tree, 
                        const Rectangle& rect1, const Rectangle& rect2,
                        const npy_float64 p,
                        npy_float64 *min, npy_float64 *max)
    {
        *min = 0.;
        *max = 0.;
        for(npy_intp i=0; i<rect1.m; ++i) {
            npy_float64 min_, max_;

            Dist1D::interval_interval(tree, rect1, rect2, i, &min_, &max_);
            min_ *= min_;
            max_ *= max_;

            *min += min_;
            *max += max_;
        }
    }
    static inline npy_float64 

    distance_p(const ckdtree * tree, 
               const npy_float64 *x, const npy_float64 *y,
               const npy_float64 p, const npy_intp k,
               const npy_float64 upperbound)
    {    
        npy_intp i;
        npy_float64 r;
        r = 0;
        for (i=0; i<k; ++i) {
            npy_float64 r1 = Dist1D::point_point(tree, x, y, i);
            r += r1 * r1;
            if (r>upperbound)
                return r;
        }
        return r;
    }
};

typedef BaseMinkowskiDistPp<Dist1D> MinkowskiDistPp;
typedef BaseMinkowskiDistPinf<Dist1D> MinkowskiDistPinf;
typedef BaseMinkowskiDistP1<Dist1D> MinkowskiDistP1;
typedef BaseMinkowskiDistP2<Dist1D> NonOptimizedMinkowskiDistP2;

struct MinkowskiDistP2: NonOptimizedMinkowskiDistP2 {
    static inline npy_float64 
    distance_p(const ckdtree * tree, 
               const npy_float64 *x, const npy_float64 *y,
               const npy_float64 p, const npy_intp k,
               const npy_float64 upperbound)
    {    
        return sqeuclidean_distance_double(x, y, k);
    }
};
