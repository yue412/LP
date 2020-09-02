#ifndef LP_INT_H
#define LP_INT_H

#include "LP.h"

class LP_Int : public LP
{
public:
    LP_Int();
    virtual ~LP_Int();
protected:
    virtual int solve(CMatrix& matrix, CVectorInt& base, CVector& result, double& objective_value);
    virtual bool adjust_matrix(CMatrix& matrix, CVectorInt& base, int col, double int_val);
    int solve_int(CMatrix& matrix, CVectorInt& base, bool check_in_base, CVector& result, double& objective_value);
private:
    int solve_int_part(CMatrix matrix, CVectorInt base, int col, double int_val, CVector& result, double& objective_value);
    bool in_base(CVectorInt& base, int col);
};

#endif // !LP_INT_H