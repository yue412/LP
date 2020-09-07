#include "LP_Int_fast.h"
#include "Common\QProfile.h"
#include <memory>


LP_Int_fast::LP_Int_fast()
{
}


LP_Int_fast::~LP_Int_fast()
{
}

int LP_Int_fast::solve(CObjectiveFunc & objective_function, CConstraintList & constraint_list, CResult & result, double & objective_value)
{
    m_oBase.clear();
    m_oBase.resize(constraint_list.size());
    return LP::solve(objective_function, constraint_list, result, objective_value);
}

int LP_Int_fast::solve(CVector & Ct, CMatrix & A, CVector & b, CVector & result, double & objective_value)
{
    QP_FUN("LP_Int_fast::solve");
    std::shared_ptr<CMatrix> pmatrix(init_matrix(Ct, A, b));
    CMatrix& matrix = *(pmatrix.get());
    //CVectorInt base(matrix.size() - 1);

    return solve(matrix, m_oBase, result, objective_value);
}

int LP_Int_fast::solve(CMatrix & matrix, CVectorInt & base, CVector & result, double & objective_value)
{
    QP_FUN("LP_Int_fast::solve2");
    CMatrix _matrix = matrix;
    CVectorInt _base = base;
    auto r = simplex(_matrix, _base, result, objective_value);
    if (r != 1)
        return r;
    return solve_int(matrix, base, true, result, objective_value);
}

bool LP_Int_fast::adjust_matrix(CMatrix & matrix, CVectorInt & base, int col, double int_val)
{
    QP_FUN("LP_Int_fast::adjust_matrix");
    //var _lp = clone_LP_info(lp);
    auto width = (int)matrix[0].size();
    CVector new_row(width, 0.0);
    new_row[col] = 1;
    new_row[new_row.size() - 1] = int_val;
    matrix.insert(matrix.begin() + matrix.size() - 1, new_row);
    gaussian(matrix, (int)matrix.size() - 2, col);
    //base[(int)matrix.size() - 2] = col;
    base.push_back(col);
    //check
    for (auto j = 0; j < ((int)matrix.size() - 1); j++) {
        auto val = matrix[j][width - 1];
        if (val < -g_epsilon) {
            return false; // нч╫Б
        }
    }
    return true;
}

void LP_Int_fast::new_auxiliary_var_event(int constraint_index, int var_index)
{
    m_oBase[constraint_index] = var_index;
}
