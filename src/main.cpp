#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <map>
#include <algorithm>
#include <iterator>
#include <memory>
#include <cmath>
#include "Common\QProfile.h"
#include "LP_Int.h"
#include "LP_Int_fast.h"
#include "cJSON.h"
#include "Common.h"

struct CMap
{
    std::wstring name;
    std::wstring type;
    int value;
    int index;
    int count;
    std::vector<int> list;
};

CMap my_maps[] = { 
    { L"牧场", L"meat", 0, 0, 4, { 25,19,13,6,1 } },
    { L"鸡舍", L"meat", 0, 0, 4,{ 24,18,14,8,4,1 } },
    { L"猪圈", L"meat", 0, 0, 4,{ 18,12,7,5,1} },
    { L"菜棚", L"veg", 0, 0, 4,{ 25,20,15,8,4,1 } },
    { L"菜地", L"veg", 0,0, 4,{ 30,22,16,8,5,1 } },
    { L"森林", L"veg", 0,0, 4,{ 32,27,17,12,6,2 } },
    { L"作坊", L"creation", 0,0, 4,{ 26,21,16,11,5,1 } },
    { L"池塘", L"fish", 0,0, 4,{ 29,24,19,14,9,5 } }
};

std::map<std::wstring, std::wstring> g_map_short_names = { 
    { L"MC",L"牧场" },
    { L"JS",L"鸡舍" },
    { L"ZJ",L"猪圈" },
    { L"CP",L"菜棚" },
    { L"CD",L"菜地" },
    { L"SL",L"森林" },
    { L"ZF",L"作坊" },
    { L"CT",L"池塘" },
};

struct CMapResult
{
    std::wstring name;
    int value;
    std::vector<std::pair<int, int>> chefs; // id, count
};

struct CChef
{
    int id;
    int count;
    std::wstring name;
    std::map<std::wstring, int> values;
};

int my_maps_cnt = 8;

cJSON* readJson(const std::wstring& sFileName);
void calc3(std::vector<CChef>& chefs, std::vector<std::pair<std::wstring, int>>& oMapOrder, int map_size, int default_map_cnt);

void init_chefs(cJSON* json, std::vector<CChef>& chefs)
{
    auto iSize = cJSON_GetArraySize(json);
    for (int i = 0; i < iSize; i++)
    {
        auto chef = cJSON_GetArrayItem(json, i);
        auto json_value_id = cJSON_GetObjectItem(chef, "id");
        auto json_value_meat = cJSON_GetObjectItem(chef, "meat");
        auto json_value_veg = cJSON_GetObjectItem(chef, "veg");
        auto json_value_fish = cJSON_GetObjectItem(chef, "fish");
        auto json_value_creation = cJSON_GetObjectItem(chef, "creation");
        auto json_value_name = cJSON_GetObjectItem(chef, "name");
        CChef oChef;
        oChef.id = std::stoi(json_value_id->valuestring);
        oChef.count = 1;
        oChef.name = Utf8ToUnicode(json_value_name->valuestring);

        auto max_value = std::max({ json_value_meat->valueint, json_value_veg->valueint, json_value_fish->valueint, json_value_creation->valueint });
        oChef.values.insert(std::make_pair(L"meat", max_value - json_value_meat->valueint > 1 ? 0 : json_value_meat->valueint));
        oChef.values.insert(std::make_pair(L"veg", max_value - json_value_veg->valueint > 1 ? 0 : json_value_veg->valueint));
        oChef.values.insert(std::make_pair(L"fish", max_value - json_value_fish->valueint > 1 ? 0 : json_value_fish->valueint));
        oChef.values.insert(std::make_pair(L"creation", max_value - json_value_creation->valueint > 1 ? 0 : json_value_creation->valueint));

        //oChef.values.insert(std::make_pair(L"meat", json_value_meat->valueint));
        //oChef.values.insert(std::make_pair(L"veg", json_value_veg->valueint));
        //oChef.values.insert(std::make_pair(L"fish", json_value_fish->valueint));
        //oChef.values.insert(std::make_pair(L"creation", json_value_creation->valueint));
        chefs.push_back(oChef);
    }
}

void filter_chefs(std::vector<CChef>& chefs, std::wstring sExclude)
{
    CStringList oList;
    split(sExclude, L',', oList);
    for each (auto sName in oList)
    {
        auto itr = std::find_if(chefs.begin(), chefs.end(), [sName](CChef& chef) {return chef.name == sName; });
        if (itr != chefs.end())
        {
            chefs.erase(itr);
        }
    }
}

void init_map_order(std::wstring sOrder, std::vector<std::pair<std::wstring, int>>& oMapOrder)
{
    CStringList oList;
    split(sOrder, L',', oList);
    for (auto i = 0; i < (int)oList.size(); i++)
    {
        CStringList oStrList;
        split(oList[i], L'-', oStrList);
        auto itr = g_map_short_names.find(UpperString( oStrList[0]));
        if (itr != g_map_short_names.end())
        {
            auto sName = itr->second;
            auto n = oStrList.size() > 1 ? std::stoi(oStrList[1]) : 0;
            oMapOrder.push_back(std::make_pair(sName, n));
        }
    }
}

void calc_money(cJSON* chefs, cJSON* recipes, cJSON* recipe_chef_price, double time_limit);

int main(int argc, char* argv[])
{
    auto json = readJson(getFullPath(L"chefs.json"));
    auto recipes_json = readJson(getFullPath(L"recipes.json"));
    auto price_json = readJson(getFullPath(L"recipe-chef-price.json"));
    std::map<std::wstring, std::wstring> oParamsMap;
    while (true)
    {
        std::string s;
        std::cout << ">";
        std::cin >> s;
        if (s.empty())
            continue;
        std::wstring str = LowerString(ToUnicode(s));

        if (str == L"play")
        {
            std::vector<CChef> chefs;
            std::vector<std::pair<std::wstring, int>> oMapOrder;
            init_chefs(json, chefs);
            filter_chefs(chefs, oParamsMap[L"exclude"]);
            init_map_order(oParamsMap[L"order"], oMapOrder);
            auto nMapSize = oParamsMap.find(L"size") == oParamsMap.end() ? 3 : std::stoi(oParamsMap[L"size"]);
            auto nMapCount = oParamsMap.find(L"count") == oParamsMap.end() ? 4 : std::stoi(oParamsMap[L"count"]);
            calc3(chefs, oMapOrder, nMapSize, nMapCount);
        }
        else if (str == L"money")
        {
            auto time_limit = oParamsMap.find(L"time_limit") == oParamsMap.end() ? 0.0 : std::stod(oParamsMap[L"time_limit"]);
            calc_money(json, recipes_json, price_json, time_limit*3600);

        }
        else if (str == L"clear")
        {
            oParamsMap.clear();
        }
        else
        {
            CStringList oStringList;
            split(str, L'=', oStringList);
            if (oStringList.size() > 1)
            {
                oParamsMap[oStringList[0]] = oStringList[1];
            }
        }
    }
    cJSON_Delete(json);

    return 0;
}

std::vector<std::string> g_cook_types = { "stirfry", "boil", "knife", "fry", "bake", "steam" };


int calc_rate(cJSON* recipe, cJSON* chef)
{
    auto n = 4;
    for (auto i = 0; i < g_cook_types.size(); i++) {
        auto type = g_cook_types[i];
        auto val = cJSON_GetObjectItem(recipe, type.c_str());
        if (val->valueint > 0)
        {
            auto chef_val = cJSON_GetObjectItem(chef, type.c_str());
            n = std::min(n, (int)floor(chef->valueint / (double)recipe->valueint));
        }
    }
    return n;
}

std::wstring display_time(int time)
{
    auto hour = time / 3600;
    time = time - hour * 3600;
    auto minute = time / 60;
    auto second = time - minute * 60;
    std::wstring result = L"";
    if (hour != 0)
        result += std::to_wstring(hour) + L"小时";
    if (minute != 0)
        result += std::to_wstring(minute) + L"分";
    if (second != 0)
        result += std::to_wstring(second) + L"秒";
    return result;
}

void calc_money(cJSON* chefs, cJSON* recipes, cJSON* recipe_chef_price, double time_limit)
{
    double final_objective_value = -1;
    CResult final_result;
    std::vector<std::vector<int>> comb_list;
    auto n = cJSON_GetArraySize(chefs);
    combination(n, 3, comb_list);
    for (auto i = 0; i < comb_list.size(); i++)
    {
        QP_FUN("calc_money");
        auto comb = comb_list[i];

        CObjectiveFunc objective_function;
        objective_function.is_max = true;
        CConstraintList constraint_list;

        CConstraint constraint_time; // 时间限制
        constraint_time.opr_type = -1;
        constraint_time.value = time_limit;

        CConstraintList constraint_arr(comb.size());
        for (auto j = 0; j < constraint_arr.size(); j++) {
            constraint_arr[j].opr_type = -1;
            constraint_arr[j].value = 3;
        }
        auto recipes_cnt = cJSON_GetArraySize(recipes);
        for (auto k = 0; k < recipes_cnt; k++) {
            auto recipe = cJSON_GetArrayItem(recipes, k);
            auto chef_prices = cJSON_GetArrayItem(recipe_chef_price, k);
            CConstraint constraint; // 同一个菜只能一个人做
            constraint.opr_type = -1;
            constraint.value = 1;
            for (auto j = 0; j < comb.size(); j++) {
                auto index = comb[j];
                auto chef = cJSON_GetArrayItem(chefs, index);
                auto price = cJSON_GetArrayItem(chef_prices, index);//calc_price(recipe, chef, my_chefs, 0);
                if (price->valueint > 0)
                {
                    //auto recipe_recipeId_val = cJSON_GetObjectItem(recipe, "recipeId");
                    auto recipe_limit_val = cJSON_GetObjectItem(recipe, "limit");
                    auto recipe_time_val = cJSON_GetObjectItem(recipe, "time");
                    auto var_name = L"X_" + std::to_wstring(k) + L"_" + std::to_wstring(index);
                    objective_function.items.push_back(std::make_pair(price->valueint*recipe_limit_val->valueint, var_name));
                    constraint_time.items.push_back(std::make_pair(recipe_time_val->valueint*recipe_limit_val->valueint, var_name));
                    constraint.items.push_back(std::make_pair(1, var_name));
                    constraint_arr[j].items.push_back(std::make_pair(1, var_name));
                }
            }
            if (constraint.items.size() > 0)
                constraint_list.push_back(constraint);
        }
        for (auto j = 0; j < constraint_arr.size(); j++) {
            if (constraint_arr[j].items.size() > 0)
                constraint_list.push_back(constraint_arr[j]);
        }
        if (time_limit> g_epsilon)
        {
            constraint_list.push_back(constraint_time);
        }

        CResult oResult;
        double objective_value;
        auto lp = std::shared_ptr<LP>(new LP_Int_fast());
        auto r = lp->solve(objective_function, constraint_list, oResult, objective_value);
        if (r == 1) {
            if (objective_value > final_objective_value)
            {
                final_objective_value = objective_value;
                final_result = oResult;
            }
        }
        if (i == 0) {
            break;
        }
    }
    // 输出结果
    int nTotal = 0;
    for (auto itr = final_result.begin(); itr != final_result.end(); itr++)
    {
        auto pair = *itr;
        if (pair.second < g_epsilon)
            continue;
        if (pair.first[0] != L'X')
            continue;
        CStringList arr;
        split(pair.first, L'_', arr);
        auto recipe_index = std::stoi(ToString(arr[1]));
        auto index = std::stoi(ToString(arr[2]));
        auto chef = cJSON_GetArrayItem(chefs, index);
        auto recipe = cJSON_GetArrayItem(recipes, recipe_index);
        auto chef_price = cJSON_GetArrayItem(recipe_chef_price, recipe_index);
        auto price = cJSON_GetArrayItem(chef_price, index);

        auto chef_name = cJSON_GetObjectItem(chef, "name");
        auto recipe_name = cJSON_GetObjectItem(recipe, "name");
        auto recipe_limit = cJSON_GetObjectItem(recipe, "limit");
        auto recipe_time = cJSON_GetObjectItem(recipe, "time");

        std::cout << ToString(Utf8ToUnicode(chef_name->valuestring)).c_str() << "\t";
        std::cout << ToString(Utf8ToUnicode(recipe_name->valuestring)).c_str() << "\t";
        std::cout << pair.second << "\t";
        std::cout << recipe_limit->valueint << "\t";
        std::cout << price->valueint*recipe_limit->valueint << "\t";
        auto time = recipe_time->valueint*recipe_limit->valueint;
        nTotal += time;
        std::cout << ToString(display_time(time)).c_str() << "\t";
        std::cout << std::endl;
    }
    std::cout << "极值:" << final_objective_value <<"\t"<< ToString(display_time(nTotal)).c_str() << std::endl;// + JSON.stringify(result) + "<br>";

}

cJSON* readJson(const std::wstring& sFileName)
{
    std::fstream in;
    in.open(ToString(sFileName), std::ios::in | std::ios::binary);
    if (!in)
    {
        std::cout << "Open file [" << ToString(sFileName).c_str() << "] failed.";
        return nullptr;
    }
    // get length of file:
    in.seekg(0, in.end);
    int length = (int)in.tellg();
    in.seekg(0, in.beg);

    //std::cout << in.rdstate() << std::endl;
    char * buffer = new char[length + 1];

    std::cout << "Reading " << length << " characters... ";
    // read data as a block:
    in.read(buffer, length);

    //std::cout << in.rdstate() << std::endl;
    if (in)
        std::cout << "all characters read successfully.";
    else
        std::cout << "error: only " << in.gcount() << " could be read";
    in.close();
    auto len = in.gcount();
    buffer[len] = '\0';
    // ...buffer contains the entire file...
    auto json = cJSON_Parse(buffer);
    delete[] buffer;

    if (json == nullptr)
    {
        std::cout << "Parse json data failed.";
    }
    std::cout << std::endl;
    return json;
}

cJSON* get_chef_by_id(cJSON* my_chefs, int id)
{
    auto iSize = cJSON_GetArraySize(my_chefs);
    for (auto i = 0; i < iSize; i++)
    {
        auto chef = cJSON_GetArrayItem(my_chefs, i);
        if (nullptr == chef)
            continue;
        auto id_value = cJSON_GetObjectItem(chef, "id");
        if (std::stoi(id_value->valuestring) == id)
        {
            return chef;
        }
    }
    return nullptr;
}

std::map<int, CChef>::iterator get_chef_by_id(std::map<int, CChef>& oChefMap, int id)
{
    for (auto itr = oChefMap.begin(); itr != oChefMap.end(); itr++)
    {
        if (itr->second.id == id)
        {
            return itr;
        }
    }
    return oChefMap.end();
}

std::vector<CChef>::const_iterator get_chef_by_id(const std::vector<CChef>& oChefMap, int id)
{
    for (auto itr = oChefMap.begin(); itr != oChefMap.end(); itr++)
    {
        if (itr->id == id)
        {
            return itr;
        }
    }
    return oChefMap.end();
}

int inner_calc(std::vector<CMap>& my_maps, std::vector<CChef>& chefs, std::vector<CMapResult>& result, double& objective_value)
{
    auto iSize = (int)chefs.size();
    CObjectiveFunc objective_function;
    objective_function.is_max = false;
    CConstraintList constraint_list;
    auto my_maps_cnt = (int)my_maps.size();
    for (int j = 0; j < my_maps_cnt; j++) {
        CMap& map = my_maps[j];
        if (abs(map.value) < g_epsilon)
            continue;
        objective_function.items.push_back(std::make_pair(1, L"t" + std::to_wstring(j)));
        {
            // 作坊
            CConstraint constraint;
            constraint.opr_type = 0;
            constraint.value = map.value;
            constraint.items.push_back(std::make_pair(-1, L"t" + std::to_wstring(j)));
            for (auto i = 0; i < iSize; i++) {
                CChef& chef = chefs[i];
                auto value = chef.values[map.type];
                auto id = chef.id;
                if (value > 0)
                    constraint.items.push_back(std::make_pair(value,
                        L"X_" + std::to_wstring(id) + L"_" + std::to_wstring(j)));
            }
            constraint_list.push_back(constraint);
        }
        {
            // 作坊
            CConstraint constraint;
            constraint.opr_type = -1;
            constraint.value = map.count;
            for (auto i = 0; i < iSize; i++) {
                CChef& chef = chefs[i];
                auto value = chef.values[map.type];
                auto id = chef.id;
                if (value > 0)
                    constraint.items.push_back(std::make_pair(1,
                        L"X_" + std::to_wstring(id) + L"_" + std::to_wstring(j)));
            }
            constraint_list.push_back(constraint);
        }
    }
    for (auto i = 0; i < iSize; i++) {
        CChef& chef = chefs[i];
        auto id = chef.id;
        CConstraint constraint;
        constraint.opr_type = -1;
        constraint.value = 1;
        for (auto j = 0; j < my_maps_cnt; j++) {
            CMap& map = my_maps[j];
            if (abs(map.value) < g_epsilon)
                continue;
            auto value = chef.values[map.type];
            if (value > 0)
                constraint.items.push_back(std::make_pair(1,
                    L"X_" + std::to_wstring(id) + L"_" + std::to_wstring(j)));
        }
        if (constraint.items.size() == 0)
            continue;
        constraint_list.push_back(constraint);
    }

    g_debug_simplex_cnt = 0;
    CResult oResult;
    auto lp = std::shared_ptr<LP>(new LP_Int());
    auto r = lp->solve(objective_function, constraint_list, oResult, objective_value);
    if (r == 1) {
        std::map<std::wstring, CMapResult> oResultMap;
        for (std::size_t i = 0; i < oResult.size(); i++) {
            auto pair = oResult[i];
            if (pair.second < g_epsilon)
                continue;
            if (pair.first[0] != L'X')
                continue;
            CStringList arr;
            split(pair.first, L'_', arr);
            auto id = std::stoi(ToString(arr[1]));
            auto map_index = std::stoi(ToString(arr[2]));
            CMap& map = my_maps[map_index];
            oResultMap[map.name].name = map.name;
            oResultMap[map.name].value = map.value;
            oResultMap[map.name].chefs.push_back(std::make_pair(id, (int)round(pair.second)));
        }
        result.clear();
        for each (auto itr in oResultMap)
        {
            result.push_back(itr.second);
        } 
    }
    return r;
}

void delete_chef_by_id(std::vector<CChef>& chefs, int id)
{
    for (size_t i = 0; i < chefs.size(); i++)
    {
        if (chefs[i].id == id)
        {
            chefs.erase(chefs.begin() + i);
            break;
        }
    }
}

void clear_chef(std::vector<CChef>& chefs, std::vector<CMapResult>& result)
{
    for each (auto i in result)
    {
        for each(auto j in i.chefs)
        {
            auto id = j.first;
            delete_chef_by_id(chefs, id);
        }
    }
}

int sort_map(std::vector<CMap>& my_maps, std::vector<std::pair<std::wstring, int>>& oMapOrder)
{
    std::vector<CMap> oList;
    for (int i = ((int)oMapOrder.size() - 1); i >= 0; i--)
    {
        auto sName = oMapOrder[i].first;
        auto itr = std::find_if(my_maps.begin(), my_maps.end(), [sName](CMap& oMap) {return oMap.name == sName; });
        if (itr != my_maps.end())
        {
            if (oMapOrder[i].second > 0 )
                itr->count = oMapOrder[i].second;
            oList.push_back(*itr);
            my_maps.erase(itr);
        }
    }
    for (int i = 0; i < (int)oList.size(); i++)
    {
        my_maps.insert(my_maps.begin(), oList[i]);
    }
    return (int)oList.size();
}

void adjust_chefs(std::vector<CMapResult>& oResult, std::vector<CMap>& my_maps, std::vector<CChef>& chefs)
{
    std::vector<CChef> chefs_bak = chefs;
    clear_chef(chefs_bak, oResult);
    for (int i = 0; i < (int)oResult.size(); i++)
    {
        CMapResult& result = oResult[i];
        auto sName = result.name;
        auto itr = std::find_if(my_maps.begin(), my_maps.end(), [sName](CMap& oMap) {return oMap.name == sName; });
        if (itr == my_maps.end())
            continue;
        auto sType = itr->type;
        // 转换为chef对象
        std::vector<CChef> oResultChefs(result.chefs.size());
        std::transform(result.chefs.begin(), result.chefs.end(), oResultChefs.begin(), [chefs](std::pair<int, int>& pair) {
            auto id = pair.first;
            auto chef_itr = get_chef_by_id(chefs, id);
            return *chef_itr;
        });
        std::vector<CChef> new_chefs;
        for (int i = ((int)oResultChefs.size() - 1); i >= 0; i--)
        {
            auto itr = std::find_if(oResultChefs[i].values.begin(), oResultChefs[i].values.end(), [sType](std::pair<const std::wstring, int>& pair) {
                return pair.second > 0 && pair.first != sType;
            });
            if (itr == oResultChefs[i].values.end())
            {
                new_chefs.push_back(oResultChefs[i]);
                oResultChefs.erase(oResultChefs.begin() + i);
            }
        }
        // 把单独的chef拿出来
        std::vector<CChef> new_chef_list;
        std::copy_if(chefs_bak.begin(), chefs_bak.end(), std::back_inserter(new_chef_list), [sType](CChef& chef) {
            auto itr = std::find_if(chef.values.begin(), chef.values.end(), [sType](std::pair<const std::wstring, int>& pair) {
                return pair.second > 0 && pair.first != sType;
            });
            return itr == chef.values.end();
        });
        // 替换
        for (int i = ((int)oResultChefs.size() - 1); i >= 0; i--)
        {
            auto nValue = oResultChefs[i].values[sType];
            auto itr = std::find_if(new_chef_list.begin(), new_chef_list.end(), [nValue, sType](CChef& chef) {
                return chef.values[sType] == nValue;
            });
            if (itr == new_chef_list.end())
                continue;
            oResultChefs.erase(oResultChefs.begin() + i);
            new_chefs.push_back(*itr);
            auto id = itr->id;
            new_chef_list.erase(itr);
            auto itr2 = std::find_if(chefs_bak.begin(), chefs_bak.end(), [id](CChef& chef) {return chef.id == id; });
            chefs_bak.erase(itr2);
        }
        std::copy(new_chefs.begin(), new_chefs.end(), std::back_inserter(oResultChefs));
        result.chefs.clear();
        result.chefs.resize(oResultChefs.size());
        std::transform(oResultChefs.begin(), oResultChefs.end(), result.chefs.begin(), [](CChef& chef) {
            return std::make_pair(chef.id, 1);
        });
    }
}

void calc3(std::vector<CChef>& chefs, std::vector<std::pair<std::wstring, int>>& oMapOrder, int map_size, int default_map_cnt)
{
    int nCount = 0;
    std::vector<CMap> my_maps = {
        { L"牧场", L"meat", 0, 0, default_map_cnt,{ 25,19,13,6,1 } },
        { L"鸡舍", L"meat", 0, 0, default_map_cnt,{ 24,18,14,8,4,1 } },
        { L"猪圈", L"meat", 0, 0, default_map_cnt,{ 18,12,7,5,1 } },
        { L"菜棚", L"veg", 0, 0, default_map_cnt,{ 25,20,15,8,4,1 } },
        { L"菜地", L"veg", 0, 0, default_map_cnt,{ 30,22,16,8,5,1 } },
        { L"森林", L"veg", 0, 0, default_map_cnt,{ 32,27,17,12,6,2 } },
        { L"作坊", L"creation", 0, 0, default_map_cnt,{ 26,21,16,11,5,1 } },
        { L"池塘", L"fish", 0, 0, default_map_cnt,{ 29,24,19,14,9,5 } }
    };
    auto map_dict = my_maps;
    std::vector<CMap> maps;
    std::vector<CChef> chefs_bak = chefs;
    //init_chefs(json, chefs);
    auto nOrderCount = sort_map(my_maps, oMapOrder);
    std::vector<CMapResult> result_list;
    auto total_objective_value = 0.0;
    std::vector<CMapResult> last_result_list;
    auto last_objective_value = 0.0;
    while (my_maps.size() > 0)
    {
        if (nOrderCount <= 0)
            std::sort(my_maps.begin(), my_maps.end(), [](const CMap& map1, const CMap& map2) { return map1.list[map1.index] > map2.list[map2.index]; });
        std::vector<CMap> new_maps(maps);
        my_maps[0].value = my_maps[0].list[my_maps[0].index];
        new_maps.push_back(my_maps[0]);
        std::vector<CMapResult> oResult;
        auto objective_value = 0.0;
        auto r = inner_calc(new_maps, chefs, oResult, objective_value);
        if (r != 1)
        {
            if (++(my_maps[0].index) >= (int)my_maps[0].list.size())
            {
                my_maps.erase(my_maps.begin());
            }
        }
        else
        {
            maps.push_back(my_maps[0]);
            my_maps.erase(my_maps.begin());
            --nOrderCount;
            last_objective_value = objective_value;
            last_result_list = oResult;
            if (maps.size() >= map_size)
            {
                //
                // 删除已选择的chef
                adjust_chefs(oResult, map_dict, chefs);
                last_result_list = oResult;
                clear_chef(chefs, oResult);
                maps.clear();
                total_objective_value += last_objective_value;
                std::copy(last_result_list.begin(), last_result_list.end(), std::back_inserter(result_list));
                last_result_list.clear();
                last_objective_value = 0.0;
            }
        }
    }
    total_objective_value += last_objective_value;
    std::copy(last_result_list.begin(), last_result_list.end(), std::back_inserter(result_list));
    // 输出结果
    std::cout << "极值:" << total_objective_value << std::endl;// + JSON.stringify(result) + "<br>";
    for (auto itr = result_list.begin(); itr != result_list.end(); itr++)
    {
        std::cout << ToString(itr->name).c_str() << "(" << itr->value <<"):";
        for (auto i = itr->chefs.begin(); i != itr->chefs.end(); i++)
        {
            auto id = i->first;
            auto itr = get_chef_by_id(chefs_bak, id);
            std::cout << ToString(itr->name).c_str() << ",";
        }
        std::cout << std::endl;
    }
}
