#ifndef LP_INT_FAST_H
#define LP_INT_FAST_H

#include "LP_Int.h"

class LP_Int_fast : public LP_Int
{
public:
    LP_Int_fast();
    virtual ~LP_Int_fast();
    virtual int solve(CObjectiveFunc& objective_function, CConstraintList& constraint_list, CResult& result, double& objective_value);
protected:
    virtual int solve(CVector& Ct, CMatrix& A, CVector& b, CVector& result, double& objective_value);
    virtual int solve(CMatrix& matrix, CVectorInt& base, CVector& result, double& objective_value);

    virtual bool adjust_matrix(CMatrix& matrix, CVectorInt& base, int col, double int_val);
    virtual void new_auxiliary_var_event(int constraint_index, int var_index);
private:
    CVectorInt m_oBase;
};

#endif // !LP_INT_FAST_H