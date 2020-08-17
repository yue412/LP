#include "LP.h"
#include <memory>
#include <string>
#include <map>
#include "Common\QProfile.h"

int g_debug_simplex_cnt = 0;
int g_debug_gaussian_cnt = 0;

CMatrix* init_matrix(CVector& Ct, CMatrix& A, CVector& b);
void init_number_array(CVector& array);
void gaussian(CMatrix& matrix, int row, int col);
bool gaussian(CMatrix& matrix, CVectorInt& base, int col);
int index_of_val_list(const CStringList& var_list, const std::wstring& var_name);
void grow_number_array(CVector& array, int new_cnt);
int simplex(CVector& Ct, CMatrix& A, CVector& b, CVector& result, double& objective_value);
int simplex(CMatrix& matrix, CVectorInt& base, CVector& result, double& objective_value);
int simplex2(CMatrix& matrix, CVectorInt& base, CVector& result, double& objective_value);

int solve_int3(CMatrix& matrix, CVectorInt& base, CVector& result, double& objective_value)
{
    QP_FUN("solve_int3");
    auto r = simplex2(matrix, base, result, objective_value);
    if (r != 1)
        return r;
    for (std::size_t i = 0; i < result.size(); i++) {
        //auto pair = result[i];
        auto int_val = (int)round(result[i]);
        if (std::abs(int_val - result[i]) < g_epsilon)
            continue;
        int_val = (int)ceil(result[i]);
        // 非整数
        //auto s = JSON.stringify(constraint_list);
        //auto new_constraint_list1 = constraint_list;

        CVectorInt base1 = base;
        base1.push_back(i);
        CMatrix matrix1 = matrix;
        matrix1.insert(matrix1.begin() + matrix1.size() - 1, CVector(matrix1[0].size(), 0.0));
        CVector& new_row = matrix1[matrix1.size() - 2];
        new_row[i] = 1;
        new_row[matrix1[0].size() - 1] = int_val;
        gaussian(matrix1, base1, i);

        CVector new_result1(matrix1[0].size() - 1);
        double new_value1;
        auto r1 = solve_int3(matrix1, base1, new_result1, new_value1);
        // 非整数
        CVectorInt base2 = base;
        base2.push_back(i);
        CMatrix matrix2 = matrix;
        matrix2.insert(matrix2.begin() + matrix2.size() - 1, CVector(matrix2[0].size(), 0.0));
        CVector& new_row = matrix2[matrix2.size() - 2];
        new_row[i] = 1;
        new_row[matrix2[0].size() - 1] = int_val - 1;
        gaussian(matrix2, base2, i);

        CVector new_result2(matrix2[0].size() - 1);
        double new_value2;
        auto r2 = solve_int3(matrix2, base2, new_result2, new_value2);
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
    return r;
}

int solve_int2(CVector& Ct, CMatrix& A, CVector& b, CVector& result, double& objective_value)
{
    QP_FUN("solve_int2");
    auto r = simplex(Ct, A, b, result, objective_value);
    if (r != 1)
        return r;
    for (std::size_t i = 0; i < result.size(); i++) {
        //auto pair = result[i];
        auto int_val = (int)round(result[i]);
        if (std::abs(int_val - result[i]) < g_epsilon)
            continue;
        int_val = (int)ceil(result[i]);
        // 非整数
        //auto s = JSON.stringify(constraint_list);
        //auto new_constraint_list1 = constraint_list;
        
        CMatrix new_A = A;
        new_A.push_back(CVector(Ct.size(), 0.0));
        CVector& new_row = new_A[new_A.size() - 1];
        new_row[i] = 1;

        CVector b1 = b;
        b1.push_back(int_val);

        CVector new_result1(Ct.size());
        double new_value1;
        auto r1 = solve_int2(Ct, new_A, b1, new_result1, new_value1);
        // 非整数
        CVector b2 = b;
        b2.push_back(int_val - 1);

        CVector new_result2(Ct.size());
        double new_value2;
        auto r2 = solve_int2(Ct, new_A, b2, new_result2, new_value2);
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
    return r;
}

int solve_int2(CObjectiveFunc& objective_function, CConstraintList& constraint_list, CResult& result, double& objective_value)
{
    QP_FUN("solve_int2_1");

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
            //if (pair.second[0] == L't')
            //{
            //    base[i] = k;
            //}
        }
        if (constraint.opr_type != 0)
        {
            auto sVar = L"_t" + std::to_wstring(new_var_no++);
            var_list.push_back(sVar);
            var_map.insert(std::make_pair(sVar, var_list.size() - 1));
            auto n = -constraint.opr_type*factor;
            row.push_back(n);
            //base[i] = var_list.size() - 1;
        }
        //A.push_back(row);
        b.push_back(std::abs(constraint.value));

        //if (row[base[i]] < -g_epsilon)
        //{
        //    for (size_t k = 0; k < row.size(); k++)
        //    {
        //        row[k] = -row[k];
        //    }
        //    b[i] = -b[i];
        //}
    }
    grow_number_array(Ct, var_list.size());
    for (std::size_t i = 0; i < A.size(); i++) {
        grow_number_array(A[i], var_list.size());
    }
    CVector X(var_list.size());
    auto r = solve_int2(Ct, A, b, X, objective_value);
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

int solve_int(CObjectiveFunc& objective_function, CConstraintList& constraint_list, CResult& result, double& objective_value)
{
    QP_FUN("solve_int");
    auto r = solve(objective_function, constraint_list, result, objective_value);
    if (r != 1)
        return r;
    for (std::size_t i = 0; i < result.size(); i++) {
        auto pair = result[i];
        auto int_val = (int)round(pair.second);
        if (std::abs(int_val - pair.second) < g_epsilon)
            continue;
        int_val = (int)ceil(pair.second);
        // 非整数
        //auto s = JSON.stringify(constraint_list);
        auto new_constraint_list1 = constraint_list;
        {
            // 作坊
            CConstraint constraint;
            constraint.opr_type = 0;
            constraint.value = int_val;
            constraint.items.push_back(std::make_pair(1, pair.first));
            new_constraint_list1.push_back(constraint);
        }
        CResult new_result1;
        double new_value1;
        auto r1 = solve_int(objective_function, new_constraint_list1, new_result1, new_value1);
        // 非整数
        auto new_constraint_list2 = constraint_list;
        {
            // 作坊
            CConstraint constraint;
            constraint.opr_type = 0;
            constraint.value = int_val - 1;
            constraint.items.push_back(std::make_pair(1, pair.first));
            new_constraint_list2.push_back(constraint);
        }
        CResult new_result2;
        double new_value2;
        auto r2 = solve_int(objective_function, new_constraint_list2, new_result2, new_value2);
        if (r1 == -1 || r2 == -1) {
            return -1;
        }
        if (r1 == 1 || r2 == 1) {
            if (r1 == 1 && r2 == 1) {
                if (((int)round(new_value1) >= (int)round(new_value2) && objective_function.is_max) ||
                    ((int)round(new_value1) <= (int)round(new_value2) && !objective_function.is_max))
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
    return r;
}

int solve(CObjectiveFunc& objective_function, CConstraintList& constraint_list, CResult& result, double& objective_value)
{
    QP_FUN("solve");

    CStringList var_list;
    std::map<std::wstring, int> var_map;
    CVector Ct;
    CVector b;
    CMatrix A;
    auto factor = objective_function.is_max ? 1 : -1;
    for (std::size_t i = 0; i < objective_function.items.size(); i++) {
        auto pair = objective_function.items[i];
        var_list.push_back(pair.second); // 变量名
        var_map.insert(std::make_pair(pair.second, i));
        Ct.push_back(pair.first * factor); // 系数
    }
    auto new_var_no = 0;
    auto nCnt = constraint_list.size();
    A.resize(nCnt);
    for (std::size_t i = 0; i < nCnt; i++) {
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
            row.push_back(-constraint.opr_type*factor);
        }
        //A.push_back(row);
        b.push_back(std::abs(constraint.value));
    }
    grow_number_array(Ct, var_list.size());
    for (std::size_t i = 0; i < A.size(); i++) {
        grow_number_array(A[i], var_list.size());
    }
    CVector X(var_list.size());
    auto r = simplex(Ct, A, b, X, objective_value);
    if (r == 1) {
        for (std::size_t i = 0; i < var_list.size(); i++) {
            if (var_list[i][0] == '_')
                continue;
            result.push_back( std::make_pair(var_list[i], X[i]));
        }
        if (!objective_function.is_max)
            objective_value = -objective_value;
    }
    return r;
}

// 单纯形法
// max(Ct*X)
// AX=b
// b>=0 X>=0
int simplex(CVector& Ct, CMatrix& A, CVector& b, CVector& result, double& objective_value)
{
    std::shared_ptr<CMatrix> pmatrix(init_matrix(Ct, A, b));
    CMatrix& matrix = *(pmatrix.get());
    CVectorInt base(matrix.size() - 1);
    return simplex(matrix, base, result, objective_value);
}

int simplex(CMatrix& matrix, CVectorInt& base, CVector& result, double& objective_value)
{
    QP_FUN("simplex");

    ++g_debug_simplex_cnt;
    // 构造Matrix
    // ( A,b)
    // (Ct,0)
    auto width = matrix[0].size() - 1;
    auto height = matrix.size();

    // 找到最初的基变量
    for (std::size_t i = 0; i < height; i++) {
        base[i] = -1;
        auto bZero = std::abs(matrix[i][width]) < g_epsilon;
        auto bSucc = false;
        for (std::size_t j = 0; j < width; j++) {
            if (std::abs(matrix[i][j]) < g_epsilon)
                continue;
            if (bZero || (matrix[i][j] > g_epsilon && matrix[i][width] > g_epsilon) || (matrix[i][j] < -g_epsilon && matrix[i][width] < -g_epsilon))
            {
                {
                    QP_FUN("simplex_gaussian_1");
                    gaussian(matrix, i, j);
                }
                base[i] = j;
                bSucc = true;
                break;
            }
        }
        if (!bSucc && !bZero)
            return 0; // 无解
    }
    //b有可能是负的了，要处理
    while (true) {
        auto isValid = true;
        for (std::size_t i = 0; i < height; i++) {
            if (matrix[i][width]<-g_epsilon)
            {
                isValid = false;
                auto bSucc = false;
                for (std::size_t j = 0; j < width; j++) {
                    if (matrix[i][j] < -g_epsilon)
                    {
                        {
                            QP_FUN("simplex_gaussian_2");
                            gaussian(matrix, i, j);
                        }
                        base[i] = j;
                        bSucc = true;
                        break;
                    }
                }
                if (!bSucc)
                    return 0;
            }
        }
        if (isValid) {
            break;
        }
    }
    auto debug_cnt = 0;
    while (true) {
        auto max_val = 0.0;
        auto col = -1;
        for (std::size_t i = 0; i < width; i++) {
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
            for (std::size_t i = 0; i < base.size(); i++) {
                auto col = base[i];
                auto row = i;
                if (col >= 0) {
                    result[col] = matrix[row][width];
                }
            }
            objective_value = -matrix[height][width];
            return 1;
        }
        /*
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
            return -1; // 可行解无界
        }
        {
            QP_FUN("simplex_gaussian_3");
            gaussian(matrix, row, col);
        }
        base[row] = col;
        */
        if (!gaussian(matrix, base, col))
            return -1; // 可行解无界
        if (debug_cnt++ > 1000) {
            return -2; // 死循环了
        }
    }
}

int simplex2(CMatrix& matrix, CVectorInt& base, CVector& result, double& objective_value)
{
    QP_FUN("simplex");

    // 构造Matrix
    // ( A,b)
    // (Ct,0)
    auto width = matrix[0].size() - 1;
    auto height = matrix.size();

    auto debug_cnt = 0;
    while (true) {
        auto max_val = 0.0;
        auto col = -1;
        for (std::size_t i = 0; i < width; i++) {
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
            for (std::size_t i = 0; i < base.size(); i++) {
                auto col = base[i];
                auto row = i;
                if (col >= 0) {
                    result[col] = matrix[row][width];
                }
            }
            objective_value = -matrix[height][width];
            return 1;
        }
        if (!gaussian(matrix, base, col))
            return -1; // 可行解无界
        if (debug_cnt++ > 1000) {
            return -2; // 死循环了
        }
    }
}

bool gaussian(CMatrix& matrix, CVectorInt& base, int col)
{
    QP_FUN("gaussian2");

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
    gaussian(matrix, row, col);
    base[row] = col;
    return true;
}

void gaussian(CMatrix& matrix, int row, int col)
{
    QP_FUN("gaussian");

    g_debug_gaussian_cnt++;
    CVector& curr_row_data = matrix[row];
    auto d = curr_row_data[col];
    if (std::abs(d) < g_epsilon)
        return;
    auto height = matrix.size();
    auto width = matrix[0].size();
    // 归一
    for (std::size_t i = 0; i < width; i++) {
        curr_row_data[i] /= d;
    }
    // 高斯消元
    for (std::size_t i = 0; i < height; i++) {
        if (i == row)
            continue;
        //        if(Math.abs(r[col]) < g_epsilon)
        //            continue;
        CVector& row_data = matrix[i];
        auto factor = row_data[col];
        for (std::size_t j = 0; j < width; j++) {
            row_data[j] -= curr_row_data[j] * factor;
        }
    }
}

CMatrix* init_matrix(CVector& Ct, CMatrix& A, CVector& b)
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

int index_of_val_list(const CStringList& var_list, const std::wstring& var_name)
{
    for (std::size_t i = 0; i < var_list.size(); i++) {
        if (var_list[i] == var_name)
            return i;
    }
    return -1;
}