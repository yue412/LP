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

class LP
{
public:
    LP();
    virtual ~LP();
    virtual int solve(CObjectiveFunc& objective_function, CConstraintList& constraint_list, CResult& result, double& objective_value);
protected:
    virtual int solve(CVector& Ct, CMatrix& A, CVector& b, CVector& result, double& objective_value);
    virtual int solve(CMatrix& matrix, CVectorInt& base, CVector& result, double& objective_value);
    virtual void new_auxiliary_var_event(int constraint_index, int var_index);
protected:
    CMatrix* init_matrix(CVector& Ct, CMatrix& A, CVector& b);
    int simplex(CMatrix& matrix, CVectorInt& base, CVector& result, double& objective_value);
    bool single_simplex(CMatrix& matrix, CVectorInt& base, int col);
    bool find_base(CMatrix& matrix, CVectorInt& base, int row);
    void row_transform(CMatrix& matrix, int row1, int row2, double factor);
    void row_transform(CMatrix& matrix, int row, double factor);
    bool adjust_nonnegative(CMatrix& matrix, CVectorInt& base);
    bool find_bases(CMatrix& matrix, CVectorInt& base);
    void gaussian(CMatrix& matrix, int row, int col);
};

#endif // !LP_H

