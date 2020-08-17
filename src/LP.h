#ifndef LP_H
#define LP_H

#include <vector>

static double g_epsilon = 1e-6;
extern int g_debug_simplex_cnt;
extern int g_debug_gaussian_cnt;

struct CConstraint;

typedef std::vector<double> CVector;
typedef std::vector<int> CVectorInt;
typedef std::vector<std::wstring> CStringList;
typedef std::vector<std::vector<double>> CMatrix;
typedef std::pair<double, std::wstring> CFuncItem;
typedef std::pair<std::wstring, double> CResultItem;
typedef std::vector<CFuncItem> CFuncItemList;
typedef std::vector<CConstraint> CConstraintList;
typedef std::vector<CResultItem> CResult;

struct CObjectiveFunc
{
    bool is_max;
    CFuncItemList items;
};

struct CConstraint
{
    int opr_type;
    double value;
    CFuncItemList items;
};

// µ¥´¿ÐÎ·¨
// max(Ct*X)
// AX=b
// b>=0 X>=0
int simplex(CVector& Ct, CMatrix& A, CVector& b, CVector& result, double& objective_value);
int solve(CObjectiveFunc& objective_function, CConstraintList& constraint_list, CResult& result, double& objective_value);
int solve_int(CObjectiveFunc& objective_function, CConstraintList& constraint_list, CResult& result, double& objective_value);
int solve_int2(CObjectiveFunc& objective_function, CConstraintList& constraint_list, CResult& result, double& objective_value);

#endif // !LP_H

