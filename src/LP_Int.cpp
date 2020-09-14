#include "LP_Int.h"
#include "Common\QProfile.h"

LP_Int::LP_Int()
{
}


LP_Int::~LP_Int()
{
}

int LP_Int::solve(CMatrix & matrix, CVectorInt & base, CVector & result, double & objective_value)
{
    QP_FUN("solve_int3");
    auto r = simplex(matrix, base, result, objective_value);
    if (r != 1)
        return r;
    return solve_int(matrix, base, false, result, objective_value);
}

bool LP_Int::adjust_matrix(CMatrix & matrix, CVectorInt & base, int col, double int_val)
{
    base.push_back(-1);
    matrix.insert(matrix.begin() + matrix.size() - 1, CVector(matrix[0].size(), 0.0));
    int height = matrix.size() - 1;

    CVector& new_row = matrix[height - 1];
    new_row[col] = 1;
    new_row[matrix[0].size() - 1] = int_val;

    for (int j = 0; j < height; j++)
    {
        if (matrix[j][col] > g_epsilon)
        {
            row_transform(matrix, j, height - 1, -1);
            break;
        }
    }
    if (!find_base(matrix, base, height - 1))
        return false;
    if (!adjust_nonnegative(matrix, base))
        return false;
    return true;
}

int LP_Int::solve_int(CMatrix & matrix, CVectorInt & base, bool check_in_base, CVector & result, double & objective_value)
{
    QP_FUN("solve_int");
    for (std::size_t i = 0; i < result.size(); i++) {
        //auto pair = result[i];
        auto int_val = (int)round(result[i]);
        if (std::abs(int_val - result[i]) < g_epsilon)
            continue;
        if (check_in_base && in_base(base, i))
            continue;
        int_val = (int)ceil(result[i]);
        // 非整数
        //auto s = JSON.stringify(constraint_list);
        //auto new_constraint_list1 = constraint_list;

        CVector new_result1(matrix[0].size() - 1);
        double new_value1;
        auto r1 = solve_int_part(matrix, base, i, int_val, new_result1, new_value1);

        // 非整数
        CVector new_result2(matrix[0].size() - 1);
        double new_value2;
        auto r2 = solve_int_part(matrix, base, i, int_val - 1, new_result2, new_value2);
        if (r1 == -1 || r2 == -1) {
            return -1;
        }
        if (r1 == 1 || r2 == 1) {
            if (r1 == 1 && r2 == 1) {
                if ((int)round(new_value1) >= (int)round(new_value2) /*&& objective_function.is_max) ||
                                                                     ((int)round(new_value1) <= (int)round(new_value2) && !objective_function.is_max)*/)
                {
                    result = new_result1;
                    //copy_array(new_result1, result);
                    //result = JSON.parse(JSON.stringify(new_result1));
                    objective_value = new_value1;
                }
                else
                {
                    result = new_result2;
                    //copy_array(new_result2, result);
                    //result = JSON.parse(JSON.stringify(new_result2));
                    objective_value = new_value2;
                }
            }
            else if (r1 == 1) {
                result = new_result1;
                //copy_array(new_result1, result);
                //result = JSON.parse(JSON.stringify(new_result1));
                objective_value = new_value1;
            }
            else
            {
                result = new_result2;
                //copy_array(new_result2, result);
                //result = JSON.parse(JSON.stringify(new_result2));
                objective_value = new_value2;
            }
            return 1;
        }
        else {
            return 0;
        }
    }
    return 1;
}

int LP_Int::solve_int_part(CMatrix matrix, CVectorInt base, int col, double int_val, CVector & result, double & objective_value)
{
    QP_FUN("solve_int4");
    if (!adjust_matrix(matrix, base, col, int_val))
        return 0;
    return solve(matrix, base, result, objective_value);
}

bool LP_Int::in_base(CVectorInt & base, int col)
{
    for (auto i = 0; i < (int)base.size(); i++) {
        if (col == base[i])
            return true;
    }
    return false;
}
