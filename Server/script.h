#ifndef SCRIPT_H
#define SCRIPT_H

#include <fstream>
#include <stdio.h>
#include <string>
#include <stdio.h>
#include <unordered_map>
#include <list>
#include <sstream>
#include <vector>
#include <iostream>

namespace Build_Script {
    /* step对应的标识符 */
    typedef std::string stepID;

    /* bool表示当前字符串是不是命令字符串 */
    typedef std::pair<bool, std::string> Item;

    class Step {
    public:
        /* step中的命令序列 */
        std::vector<std::string> commands;
        /* 存储说的话语 */
        std::vector<Item> speak;
        /* 
            listen单位是s
            listen之后的分支，以及下一步跳转的状态 
            listen只能有一个，因为之后会跳转状态
        */
        int listen;
        /* branch分支：关键词————回答 */
        std::unordered_map<std::string, stepID> branchs;
        /* 如果沉默跳转到什么状态 */
        stepID silence;
        /* 此状态结束没有任何匹配应到什么状态 */
        stepID _default;
        /* 直接跳转到某个状态 */
        stepID _goto;
    };
    typedef std::unordered_map<stepID, Step> steps;

    /* 存储初始状态和所有步骤及内容 */
    class State {
    public:
        stepID init;
        steps _steps;
    };

    /* 脚本 */
    class Script {
    public:
        /* 存储每个stepID所对应的内容 */
        Script(const std::string& filename);
        /* 得到脚本分析树 */
        State get_State(void);
        /* 测试桩测试输出 */
        void show(void);
    private:
        /* 存储每个stepID所对应的内容 */
        State _state;

        /* 脚本当前正在处理哪个step */
        stepID _mainStepID;

        /* 私有属性及不对类外开放函数 */
        std::ifstream _input;
        void _build_GrammerTree();
        int _handle_Comments_black(std::string& line);
        void _handle_TokenLine(std::string line);
        void _process_Step(stepID id);
        void _process_Speak(std::vector<std::string>& tokens);
        void _process_Listen(std::string& time);
        void _process_Branch(std::string token1, std::string token2);
        void _process_Silence(stepID id);
        void _process_Default(stepID id);
        void _process_Exit();
        void _process_Goto(stepID id);
    };
};

#endif