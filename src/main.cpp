#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <map>
#include <algorithm>
#include <iterator>
#include "LP.h"
#include "cJSON.h"
#include "Common.h"

struct CMap
{
    std::wstring name;
    std::wstring type;
    int value;
    int index;
    std::vector<int> list;
};

CMap my_maps[] = { 
    { L"牧场", L"meat", 0, 0, { 25,19,13,6,1 } },
    { L"鸡舍", L"meat", 0, 0,{ 24,18,14,8,4,1 } },
    { L"猪圈", L"meat", 0, 0,{ 18,12,7,5,1} },
    { L"菜棚", L"veg", 0, 0,{ 25,20,15,8,4,1 } },
    { L"菜地", L"veg", 0,0,{ 30,22,16,8,5,1 } },
    { L"森林", L"veg", 0,0,{ 32,27,17,12,6,2 } },
    { L"作坊", L"creation", 0,0,{ 26,21,16,11,5,1 } },
    { L"池塘", L"fish", 0,0,{ 29,24,19,14,9,5 } }
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
    std::map<std::wstring, int> values;
};

int my_maps_cnt = 8;

cJSON* readJson(const std::wstring& sFileName);
void calc(CMap* my_maps, cJSON* json);
void calc1(CMap* my_maps, cJSON* json);
void calc3(cJSON* json);

int main(int argc, char* argv[])
{
    auto json = readJson(getFullPath(L"chefs.json"));
    while (true)
    {
        std::string s;
        std::cout << ">";
        std::cin >> s;
        if (s.empty())
            continue;
        std::wstring str = ToUnicode(s);
        CStringList oStringList;
        split(str, L',', oStringList);
        for (int i = 0; i < my_maps_cnt && i < (int)oStringList.size(); i++)
        {
            my_maps[i].value = std::stoi(oStringList[i]);
        }

        //calc(my_maps, json);
        //calc1(my_maps, json);
        calc3(json);
    }
    cJSON_Delete(json);

    return 0;
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

std::vector<CChef>::iterator get_chef_by_id(std::vector<CChef>& oChefMap, int id)
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


void calc(CMap* my_maps, cJSON* json)
{
    auto iSize = cJSON_GetArraySize(json);
    CObjectiveFunc objective_function;
    objective_function.is_max = false;
    CConstraintList constraint_list;
    CResult result;
    double objective_value;
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
                auto chef = cJSON_GetArrayItem(json, i);
                if (NULL == chef)
                    continue;
                auto json_value = cJSON_GetObjectItem(chef, ToString(map.type).c_str());
                auto json_id_value = cJSON_GetObjectItem(chef, "id");
                if (json_value->valueint > 0)
                    constraint.items.push_back(std::make_pair(json_value->valueint, 
                        L"X_" + Utf8ToUnicode(json_id_value->valuestring) + L"_" + std::to_wstring(j)));
            }
            constraint_list.push_back(constraint);
        }
        {
            // 作坊
            CConstraint constraint;
            constraint.opr_type = -1;
            constraint.value = 5;
            for (auto i = 0; i < iSize; i++) {
                auto chef = cJSON_GetArrayItem(json, i);
                if (NULL == chef)
                    continue;
                auto json_value = cJSON_GetObjectItem(chef, ToString(map.type).c_str());
                auto json_id_value = cJSON_GetObjectItem(chef, "id");
                if (json_value->valueint > 0)
                    constraint.items.push_back(std::make_pair(1, 
                        L"X_" + Utf8ToUnicode(json_id_value->valuestring) + L"_" + std::to_wstring(j)));
            }
            constraint_list.push_back(constraint);
        }
    }
    for (auto i = 0; i < iSize; i++) {
        auto chef = cJSON_GetArrayItem(json, i);
        if (NULL == chef)
            continue;
        auto json_id_value = cJSON_GetObjectItem(chef, "id");
        CConstraint constraint;
        constraint.opr_type = -1;
        constraint.value = 1;
        for (auto j = 0; j < my_maps_cnt; j++) {
            CMap& map = my_maps[j];
            if (abs(map.value) < g_epsilon)
                continue;
            auto json_value = cJSON_GetObjectItem(chef, ToString(map.type).c_str());
            if (json_value->valueint > 0)
                constraint.items.push_back(std::make_pair(1, 
                    L"X_" + Utf8ToUnicode(json_id_value->valuestring) + L"_" + std::to_wstring(j)));
            //if (chef[map.type] > 0)
            //    constraint.items.push_back([1, "X_" + chef.id + "_" + j]);
        }
        if (constraint.items.size() == 0)
            continue;
        constraint_list.push_back(constraint);
    }

    g_debug_simplex_cnt = 0;
    auto r = solve_int(objective_function, constraint_list, result, objective_value);
    if (r == 1) {
        std::cout << "复杂度:" << g_debug_simplex_cnt << std::endl;
        std::cout << "极值:" << objective_value << std::endl;// + JSON.stringify(result) + "<br>";
        for (auto i = 0; i < my_maps_cnt; i++) {
            CMap& map = my_maps[i];
            if (abs(map.value) < g_epsilon)
                continue;
            std::cout << ToString(map.name).c_str() << "(" << map.value << "),";
        }
        std::cout << std::endl;
        for (std::size_t i = 0; i < result.size(); i++) {
            auto pair = result[i];
            if (pair.second < g_epsilon)
                continue;
            if (pair.first[0] != L'X')
                continue;
            CStringList arr;
            split(pair.first, L'_', arr);
            //auto arr = pair[0].split("_");
            auto id = std::stoi(ToString(arr[1]));
            auto chef = get_chef_by_id(json, id);
            auto map_index = std::stoi(ToString(arr[2]));
            CMap& map = my_maps[map_index];
            std::cout << ToString(map.name).c_str();
            auto name_value = cJSON_GetObjectItem(chef, "name");
            std::cout << "\t" << ToString(Utf8ToUnicode(name_value->valuestring)).c_str();
            auto value = cJSON_GetObjectItem(chef, ToString(map.type).c_str());
            std::cout << "\t" << value->valueint;
            std::cout << "\t" << std::setprecision(2) << pair.second << std::endl;
        }
    }
    else if (r == 0)
    {
        std::cout << "无解" << std::endl;
    }
    else {
        std::cout << "无穷解" << std::endl;
    }
}

void calc1(CMap* my_maps, cJSON* json)
{
    std::map<int, CChef> oChefMap;
    auto iSize = cJSON_GetArraySize(json);
    int nId = 0;
    for (int i = 0; i < iSize; i++)
    {
        auto chef = cJSON_GetArrayItem(json, i);
        auto json_value_meat = cJSON_GetObjectItem(chef, "meat");
        auto json_value_veg = cJSON_GetObjectItem(chef, "veg");
        auto json_value_fish = cJSON_GetObjectItem(chef, "fish");
        auto json_value_creation = cJSON_GetObjectItem(chef, "creation");
        auto key = json_value_meat->valueint +
            json_value_veg->valueint*10 +
            json_value_fish->valueint*100 +
            json_value_creation->valueint*1000;
        if (oChefMap.find(key) == oChefMap.end())
        {
            oChefMap[key].id = nId;
            oChefMap[key].count = 0;
            
            //auto max_value = std::max({ json_value_meat->valueint, json_value_veg->valueint, json_value_fish->valueint, json_value_creation->valueint });
            oChefMap[key].values.insert(std::make_pair(L"meat", json_value_meat->valueint));
            oChefMap[key].values.insert(std::make_pair(L"veg", json_value_veg->valueint));
            oChefMap[key].values.insert(std::make_pair(L"fish", json_value_fish->valueint));
            oChefMap[key].values.insert(std::make_pair(L"creation", json_value_creation->valueint));
            //oChefMap[key].values.insert(std::make_pair(L"meat", max_value - json_value_meat->valueint > 1 ? 0 : json_value_meat->valueint));
            //oChefMap[key].values.insert(std::make_pair(L"veg", max_value - json_value_veg->valueint > 1 ? 0 : json_value_veg->valueint));
            //oChefMap[key].values.insert(std::make_pair(L"fish", max_value - json_value_fish->valueint > 1 ? 0 : json_value_fish->valueint));
            //oChefMap[key].values.insert(std::make_pair(L"creation", max_value - json_value_creation->valueint > 1 ? 0 : json_value_creation->valueint));
            ++nId;
        }
        oChefMap[key].count += 1;
    }
    CObjectiveFunc objective_function;
    objective_function.is_max = false;
    CConstraintList constraint_list;
    CResult result;
    double objective_value;
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
            for (auto itr = oChefMap.begin(); itr != oChefMap.end(); itr++) {
                auto value = itr->second.values[map.type];
                if (value > 0)
                    constraint.items.push_back(std::make_pair(value,
                        L"X_" + std::to_wstring(itr->second.id) + L"_" + std::to_wstring(j)));
            }
            constraint.items.push_back(std::make_pair(-1, L"t" + std::to_wstring(j)));
            constraint_list.push_back(constraint);
        }
        {
            // 作坊
            CConstraint constraint;
            constraint.opr_type = -1;
            constraint.value = 5;
            for (auto itr = oChefMap.begin(); itr != oChefMap.end(); itr++) {
                auto value = itr->second.values[map.type];
                if (value > 0)
                    constraint.items.push_back(std::make_pair(1,
                        L"X_" + std::to_wstring(itr->second.id) + L"_" + std::to_wstring(j)));
            }
            constraint_list.push_back(constraint);
        }
    }
    for (auto itr = oChefMap.begin(); itr != oChefMap.end(); itr++) {
        auto json_id_value = itr->second.id;
        CConstraint constraint;
        constraint.opr_type = -1;
        constraint.value = itr->second.count;
        for (auto j = 0; j < my_maps_cnt; j++) {
            CMap& map = my_maps[j];
            if (abs(map.value) < g_epsilon)
                continue;
            auto json_value = itr->second.values[map.type];
            if (json_value > 0)
                constraint.items.push_back(std::make_pair(1,
                    L"X_" + std::to_wstring(json_id_value) + L"_" + std::to_wstring(j)));
            //if (chef[map.type] > 0)
            //    constraint.items.push_back([1, "X_" + chef.id + "_" + j]);
        }
        if (constraint.items.size() == 0)
            continue;
        constraint_list.push_back(constraint);
    }

    g_debug_simplex_cnt = 0;
    g_debug_gaussian_cnt = 0;
    auto r = solve_int(objective_function, constraint_list, result, objective_value);
    if (r == 1) {
        std::cout << "复杂度:" << g_debug_simplex_cnt << "," << g_debug_gaussian_cnt << std::endl;
        std::cout << "极值:" << objective_value << std::endl;// + JSON.stringify(result) + "<br>";
        for (auto i = 0; i < my_maps_cnt; i++) {
            CMap& map = my_maps[i];
            if (abs(map.value) < g_epsilon)
                continue;
            std::cout << ToString(map.name).c_str() << "(" << map.value << "),";
        }
        std::cout << std::endl;
        for (std::size_t i = 0; i < result.size(); i++) {
            auto pair = result[i];
            if (pair.second < g_epsilon)
                continue;
            if (pair.first[0] != L'X')
                continue;
            CStringList arr;
            split(pair.first, L'_', arr);
            //auto arr = pair[0].split("_");
            auto id = std::stoi(ToString(arr[1]));
            auto itr = get_chef_by_id(oChefMap, id);
            auto map_index = std::stoi(ToString(arr[2]));
            CMap& map = my_maps[map_index];
            std::cout << ToString(map.name).c_str();
            std::cout << "\t" << itr->first;
            auto value = itr->second.values[map.type];
            std::cout << "\t" << value;
            std::cout << "\t" << std::setprecision(2) << pair.second << std::endl;
        }
    }
    else if (r == 0)
    {
        std::cout << "无解" << std::endl;
    }
    else {
        std::cout << "无穷解" << std::endl;
    }
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
            constraint.value = 4;
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
    auto r = solve_int(objective_function, constraint_list, oResult, objective_value);
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
        CChef oChef;
        oChef.id = std::stoi(json_value_id->valuestring);
        oChef.count = 1;

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

void calc3(cJSON* json)
{
    int nCount = 0;
    std::vector<CMap> my_maps = {
        { L"牧场", L"meat", 0, 0,{ 25,19,13,6,1 } },
        { L"鸡舍", L"meat", 0, 0,{ 24,18,14,8,4,1 } },
        { L"猪圈", L"meat", 0, 0,{ 18,12,7,5,1 } },
        { L"菜棚", L"veg", 0, 0,{ 25,20,15,8,4,1 } },
        { L"菜地", L"veg", 0,0,{ 30,22,16,8,5,1 } },
        { L"森林", L"veg", 0,0,{ 32,27,17,12,6,2 } },
        { L"作坊", L"creation", 0,0,{ 26,21,16,11,5,1 } },
        { L"池塘", L"fish", 0,0,{ 29,24,19,14,9,5 } }
    };
    std::vector<CMap> maps;
    std::vector<CChef> chefs;
    init_chefs(json, chefs);
    std::vector<CMapResult> result_list;
    auto total_objective_value = 0.0;
    std::vector<CMapResult> last_result_list;
    auto last_objective_value = 0.0;
    while (my_maps.size() > 0)
    {
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
            last_objective_value = objective_value;
            last_result_list = oResult;
            if (maps.size() >= 4)
            {
                //
                // 删除已选择的chef
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
            auto chef = get_chef_by_id(json, id);
            auto name_value = cJSON_GetObjectItem(chef, "name");
            std::cout << ToString(Utf8ToUnicode(name_value->valuestring)).c_str() << ",";
        }
        std::cout << std::endl;
    }
}
