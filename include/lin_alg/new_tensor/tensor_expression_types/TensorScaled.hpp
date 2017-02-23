//
// Created by tyler on 2/21/17.
//

#ifndef YAFEL_TENSORSCALED_HPP
#define YAFEL_TENSORSCALED_HPP

#include "lin_alg/new_tensor/TensorExpression.hpp"

YAFEL_NAMESPACE_OPEN

template<typename TE, typename scaleT, int D, int R, typename dataType>
class TensorScaled : public TensorExpression<TensorScaled<TE,scaleT,D,R,dataType>,D,R,dataType,false>
{
public:
    const TE& te_ref;
    const scaleT &scale;

    template<typename dt1, typename dt2, bool b>
    TensorScaled(const TensorExpression<TE,D,R,dt1,b> &te, const dt2 &s)
            : te_ref(te.self()), scale(s)
    {}


    inline dataType linearIndexing(int idx) const noexcept
    {
        return scale*te_ref.linearIndexing(idx);
    }
};


template<typename dt1, typename TE, int D, int R, typename dt2, bool b>
auto operator*(const dt1& lhs, const TensorExpression<TE,D,R,dt2,b> &rhs)
{
    return TensorScaled<TE,dt1,D,R,decltype(dt1(0)*dt2(0))>(rhs, lhs);
};

template<typename dt1, typename TE, int D, int R, typename dt2, bool b>
auto operator*(const TensorExpression<TE,D,R,dt2,b> &lhs, const dt1& rhs)
{
    return TensorScaled<TE,dt1,D,R,decltype(dt1(0)*dt2(0))>(lhs, rhs);
};

YAFEL_NAMESPACE_CLOSE

#endif //YAFEL_TENSORSCALED_HPP