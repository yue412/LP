#include "LP.h"
#include <memory>
#include <string>
#include <map>
#include "Common\QProfile.h"

int g_debug_simplex_cnt = 0;
int g_debug_gaussian_cnt = 0;

void grow_number_array(CVector& array, int new_cnt)
{
    auto cnt = new_cnt - array.size();
    for (std::size_t i = 0; i < cnt; i++) {
        array.push_back(0);
    }
}

void init_number_array(CVector& array)
{
    for (std::size_t i = 0; i < array.size(); i++) {
        array[i] = 0;
    }
}

//int index_of_val_list(const CStringList& var_list, const std::wstring& var_name)
//{
//    for (std::size_t i = 0; i < var_list.size(); i++) {
//        if (var_list[i] == var_name)
//            return i;
//    }
//    return -1;
//}

LP::LP()
{
}

LP::~LP()
{
}

int LP::solve(CObjectiveFunc & objective_function, CConstraintList & constraint_list, CResult & result, double & objective_value)
{
    QP_FUN("solve");

    CStringList var_list;
    //CVectorInt base;
    std::map<std::wstring, int> var_map;
    CVector Ct;
    auto factor = objective_function.is_max ? 1 : -1;
    for (std::size_t i = 0; i < objective_function.items.size(); i++) {
        auto pair = objective_function.items[i];
        var_list.push_back(pair.second); // 变量名
        var_map.insert(std::make_pair(pair.second, i));
        Ct.push_back(pair.first * factor); // 系数
    }
    auto new_var_no = 0;
    auto nCnt = constraint_list.size();
    CVector b;
    CMatrix A;
    A.resize(nCnt);
    //base.resize(nCnt);
    for (std::size_t i = 0; i < nCnt; i++) {
        //base[i] = -1;
        CConstraint& constraint = constraint_list[i];
        auto factor = constraint.value < -g_epsilon ? -1 : 1;
        CVector& row = A[i];
        row.resize(var_list.size());
        //CVector row();
        init_number_array(row);
        for (std::size_t j = 0; j < constraint.items.size(); j++) {
            auto pair = constraint.items[j];
            int k = 0;
            {
                //QP_FUN("index_of_val_list");
                auto itr = var_map.find(pair.second);
                k = itr != var_map.end() ? itr->second : -1;
                //k = index_of_val_list(var_list, pair.second);
            }
            auto val = pair.first * factor;
            if (k == -1)
            {
                //var_list.push_back("_t"+(new_var_no++));
                var_list.push_back(pair.second);
                var_map.insert(std::make_pair(pair.second, var_list.size() - 1));
                //k = var_list.size() - 1;
                row.push_back(val);
            }
            else
            {
                row[k] = val;
            }
        }
        if (constraint.opr_type != 0)
        {
            auto sVar = L"_t" + std::to_wstring(new_var_no++);
            var_list.push_back(sVar);
            var_map.insert(std::make_pair(sVar, var_list.size() - 1));
            auto n = -constraint.opr_type*factor;
            row.push_back(n);
            new_auxiliary_var_event(i, var_list.size() - 1);
            //base[i] = var_list.size() - 1;
        }
        //A.push_back(row);
        b.push_back(std::abs(constraint.value));

    }
    grow_number_array(Ct, var_list.size());
    for (std::size_t i = 0; i < A.size(); i++) {
        grow_number_array(A[i], var_list.size());
    }
    CVector X(var_list.size());
    auto r = solve(Ct, A, b, X, objective_value);
    if (r == 1) {
        for (std::size_t i = 0; i < var_list.size(); i++) {
            if (var_list[i][0] == '_')
                continue;
            result.push_back(std::make_pair(var_list[i], X[i]));
        }
        if (!objective_function.is_max)
            objective_value = -objective_value;
    }
    return r;
}

int LP::solve(CVector & Ct, CMatrix & A, CVector & b, CVector & result, double & objective_value)
{
    QP_FUN("solve2");
    std::shared_ptr<CMatrix> pmatrix(init_matrix(Ct, A, b));
    CMatrix& matrix = *(pmatrix.get());
    CVectorInt base(matrix.size() - 1);

    // 找到最初的基变量
    if (!find_bases(matrix, base))
        return 0;
    //b有可能是负的了，要处理
    if (!adjust_nonnegative(matrix, base))
        return 0; //无解

    return solve(matrix, base, result, objective_value);
}

int LP::solve(CMatrix & matrix, CVectorInt & base, CVector & result, double & objective_value)
{
    return simplex(matrix, base, result, objective_value);
}

void LP::new_auxiliary_var_event(int constraint_index, int var_index)
{
}

CMatrix * LP::init_matrix(CVector & Ct, CMatrix & A, CVector & b)
{
    // 构造Matrix
    // ( A,b)
    // (Ct,0)
    auto width = Ct.size();
    auto height = b.size();
    CMatrix* matrix = new CMatrix(height + 1);
    for (std::size_t i = 0; i < height; i++) {
        auto row = &((*matrix)[i]);
        row->resize(width + 1);
        for (std::size_t j = 0; j < width; j++) {
            (*row)[j] = A[i][j];
        }
        (*row)[width] = b[i];
    }

    auto last_row = &(*matrix)[height];
    last_row->resize(width + 1);
    for (std::size_t i = 0; i < width; i++) {
        (*last_row)[i] = Ct[i];
    }
    (*last_row)[width] = 0;

    return matrix;
}

int LP::simplex(CMatrix & matrix, CVectorInt & base, CVector & result, double & objective_value)
{
    QP_FUN("simplex");
    auto width = (int)matrix[0].size() - 1;
    auto height = (int)matrix.size() - 1;
    auto debug_cnt = 0;
    while (true) {
        auto max_val = 0.0;
        auto col = -1;
        for (int i = 0; i < width; i++) {
            if (matrix[height][i] >(max_val + g_epsilon))
            {
                max_val = matrix[height][i];
                col = i;
            }
        }
        if (col == -1) {
            // 有解
            // 非基变量全部为0
            // auto result = new Array(b.length);
            init_number_array(result);
            for (int i = 0; i < (int)base.size(); i++) {
                auto col = base[i];
                auto row = i;
                if (col >= 0) {
                    result[col] = matrix[row][width];
                }
            }
            objective_value = -matrix[height][width];
            return 1;
        }
        if (!single_simplex(matrix, base, col))
            return -1; // 可行解无界
        if (debug_cnt++ > 1000) {
            return -2; // 死循环了
        }
    }
}

bool LP::single_simplex(CMatrix & matrix, CVectorInt & base, int col)
{
    QP_FUN("single_simplex");

    // 构造Matrix
    // ( A,b)
    // (Ct,0)
    auto width = matrix[0].size() - 1;
    auto height = matrix.size() - 1;

    auto min_val = 0.0;
    auto row = -1;
    for (std::size_t i = 0; i < height; i++) {
        if (std::abs(matrix[i][col]) < g_epsilon)
            continue;
        if (matrix[i][col] > g_epsilon) {
            auto f = matrix[i][width] / matrix[i][col];
            if ((row == -1) || (f < min_val - g_epsilon)) {
                min_val = f;
                row = i;
            }
        }
    }
    if (row == -1) {
        return false; // 可行解无界
    }
    {QP_FUN("single_simplex_gaussian");
    gaussian(matrix, row, col);
    }
    base[row] = col;
    return true;
}

bool LP::find_base(CMatrix & matrix, CVectorInt & base, int row)
{
    auto width = (int)matrix[row].size() - 1;//不含最后一列b
    auto bZero = std::abs(matrix[row][width]) < g_epsilon;
    auto bMin = 0.0;
    base[row] = -1;
    for (auto j = 0; j < width; j++) {
        if (std::abs(matrix[row][j]) < g_epsilon)
            continue;
        if (bZero || (matrix[row][j] > g_epsilon && matrix[row][width] > g_epsilon)
            || (matrix[row][j] < -g_epsilon && matrix[row][width] < -g_epsilon))
        {

            if ((base[row] == -1) || abs(matrix[row][j]) < (bMin - g_epsilon))
            {
                bMin = abs(matrix[row][j]);
                base[row] = j;
            }

            /*
            QP_FUN("find_base_gaussian");
            gaussian(matrix, row, j);
            base[row] = j;
            return true;
            */

        }
    }

    if (base[row] >= 0)
    {
        QP_FUN("find_base_gaussian");
        gaussian(matrix, row, base[row]);
        return true;
    }

    return bZero;
}

//row2 += row1*factor
void LP::row_transform(CMatrix & matrix, int row1, int row2, double factor)
{
    auto width = (int)matrix[0].size();
    for (int i = 0; i < width; i++)
    {
        matrix[row2][i] += matrix[row1][i] * factor;
    }
}

// row = row*factor
void LP::row_transform(CMatrix & matrix, int row, double factor)
{
    auto width = (int)matrix[0].size();
    for (int i = 0; i < width; i++)
    {
        matrix[row][i] *= factor;
    }
}

// 调整使b非负
bool LP::adjust_nonnegative(CMatrix & matrix, CVectorInt & base)
{
    auto height = (int)matrix.size() - 1;
    auto width = (int)matrix[0].size() - 1;
    while (true) {
        auto isValid = true;
        for (auto i = 0; i < height; i++) {
            if (matrix[i][width]<-g_epsilon)
            {
                isValid = false;
                //auto bSucc = false;
                auto dMin = 0.0;
                base[i] = -1;
                for (auto j = 0; j < width; j++) {
                    if (matrix[i][j] < -g_epsilon)
                    {
                        if ((base[i] == -1) || (abs(matrix[i][j]) < (dMin - g_epsilon)))
                        {
                            dMin = abs(matrix[i][j]);
                            //gaussian(matrix, i, j);
                            base[i] = j;
                            //bSucc = true;
                        }
                        //break;
                    }
                }

                if (base[i] >= 0)
                {
                    QP_FUN("adjust_nonnegative_gaussian");
                    gaussian(matrix, i, base[i]);
                }
                else
                    return false;

                // 无解
                //if (!bSucc)
                //    return false;
            }
        }
        if (isValid) {
            return true;
        }
    }
}

bool LP::find_bases(CMatrix & matrix, CVectorInt & base)
{
    auto height = matrix.size() - 1;
    for (std::size_t i = 0; i < height; i++) {
        if (!find_base(matrix, base, i))
            return false; // 无解
    }
    return true;
}

void LP::gaussian(CMatrix & matrix, int row, int col)
{
    QP_FUN("gaussian");

    g_debug_gaussian_cnt++;
    auto d = matrix[row][col];
    if (std::abs(d) < g_epsilon)
        return;
    auto height = matrix.size();
    // 归一
    row_transform(matrix, row, 1 / d);
    // 高斯消元
    for (std::size_t i = 0; i < height; i++) {
        if (i == row)
            continue;
        row_transform(matrix, row, i, -matrix[i][col]);
    }
}
