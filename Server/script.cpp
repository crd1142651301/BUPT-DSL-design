#include "script.h"

/* 标记是否是初始步骤 */
bool start = true;

namespace Build_Script {
    Script::Script(const std::string& filename) {
        int n = filename.length();
        if (filename.substr(n-4, n) == ".txt") {
            _input.open(filename);
            if (_input.fail())
                printf("文件打开失败\n");
            else {
                // 打开成功，调用函数建立语法分析树
                _build_GrammerTree();
                printf("文件打开成功\n");
            }
        }else
            printf("脚本正确的文件后缀格式应该为.txt\n");
    }
    
    // 向类外提供一个获得语法分析树的接口
    State Script::get_State(void) {
        return _state;
    }

    void Script::show(void) 
    {
        // 测试读入是否正确
        std::cout << "初始步骤： " << _state.init << std::endl;
        Build_Script::steps _steps = _state._steps;
        for (auto it = _steps.begin() ; it != _steps.end() ; ++it)
        {
            std::cout << "-----------------" << std::endl;
            std::cout << it->first << "步骤内容如下：" << std::endl;
            Build_Script::Step step = it->second;
            for (auto command : step.commands)
            {
                if (command == "speak")
                {
                    std::cout << command << " ";
                    for (auto item : step.speak)
                    {
                        if (item.first)
                            std::cout << "$" << item.second << " ";
                        else
                            std::cout << item.second << " ";
                    }
                    std::cout << std::endl;
                }
                else if (command == "listen")
                {
                    std::cout << command << " " << step.listen << std::endl;
                    for (auto _it = step.branchs.begin(); _it != step.branchs.end() ; ++_it)
                    {
                        std::cout << "分支" << _it->first << ": " << _it->second << std::endl;
                    }
                    std::cout << "沉默分支silence: " << step.silence << std::endl;
                    std::cout << "无匹配分支default: " << step._default << std::endl;
                }
                else if (command == "goto")
                {
                    std::cout << command << " " << step._goto << std::endl;
                }
                else if (command == "exit")
                {   
                    std::cout << command << std::endl;
                }
            }
            std::cout << "-----------------" << std::endl;
        }
    }

    /* 建立语法树 */
    void Script::_build_GrammerTree(void) {
        std::string line;
        while (getline(_input, line)) {
            int index = _handle_Comments_black(line);
            if (index != -1) {
                _handle_TokenLine(line.substr(index, line.size()));
            }
        }
    }

    /* 
        若为注释行，返回-1
        若为空行，返回-1
        去除前置空格
    */
    int Script::_handle_Comments_black(std::string& line) {
        for (int i = 0; i < line.length() ; ++i) {
            if (line[i] == '#')
                return -1;
            else if (line[i] != ' ')
                return i;
        }
        return -1;
    }

    // 处理每一行token
    void Script::_handle_TokenLine(std::string line) {
        std::istringstream record(line);
        std::vector<std::string> tokens;
        std::string token;

        // 使用空格分隔token
        while (record >> token)
        {
            if (token.back() == ':')    token.pop_back();
            if (token.length() > 0)
                tokens.emplace_back(token);
        } 
        
        std::string keys[8] = {"step", "speak", "listen", "branch", "silence", "default", "goto", "exit"};
        for (int i = 0; i < 8 ; ++i) {
            if (tokens[0] == keys[i]) {
                switch (i) {
                    case 0: _process_Step(tokens[1]); break;
                    case 1: _process_Speak(tokens); break;
                    case 2: _process_Listen(tokens[1]); break;
                    case 3: _process_Branch(tokens[1], tokens[2]); break;
                    case 4: _process_Silence(tokens[1]); break;
                    case 5: _process_Default(tokens[1]); break;
                    case 6: _process_Goto(tokens[1]); break;
                    case 7: _process_Exit(); break;
                    default: printf("无%s此关键词！请检查脚本！", tokens[0].c_str()); 
                            break;
                }
                break;
            }
        }
    }
    // 处理steps
    void Script::_process_Step(stepID id) {
        Step step;
        if (start)
        {
            _state.init = id;
            start = false;
        }
        _state._steps[id] = step;
        _mainStepID = id;
        // 沉默、无匹配、跳转默认为本身状态
        _state._steps[_mainStepID]._default = id;
        _state._steps[_mainStepID].silence = id;
        _state._steps[_mainStepID]._goto = id;
    }

    // 处理说话的语句
    void Script::_process_Speak(std::vector<std::string>& tokens) {
        _state._steps[_mainStepID].commands.push_back("speak");
        for (int i = 1; i < tokens.size() ; ++i) {
            if (tokens[i] == "+")    continue;
            if (tokens[i][0] == '$')
                _state._steps[_mainStepID].speak.push_back({true, tokens[i].substr(1, tokens[i].length())});
            else if(tokens[i][0] == '"' && tokens[i][tokens[i].length() - 1] == '"')
                _state._steps[_mainStepID].speak.push_back({false, tokens[i].substr(1, tokens[i].length() - 2)});
            else
                printf("speak语句中%s不符合规范", tokens[i].c_str());
        }
    }

    // 处理听的语句
    void Script::_process_Listen(std::string& time) {
        _state._steps[_mainStepID].commands.push_back("listen");
        _state._steps[_mainStepID].listen = stoi(time);
    }
    // 将分支语句存入Hashtable
    void Script::_process_Branch(std::string token1, std::string token2) {
        token1 = token1.substr(1, token1.length() - 2);
        _state._steps[_mainStepID].branchs[token1] = token2;
    }
    // 处理沉默语句
    void Script::_process_Silence(stepID id) {
        _state._steps[_mainStepID].silence = id;
    }
    // 处理无匹配语句
    void Script::_process_Default(stepID id) {
        _state._steps[_mainStepID]._default = id;
    }
    // 处理直接跳转状态语句
    void Script::_process_Goto(stepID id) {
        _state._steps[_mainStepID].commands.push_back("goto");
        _state._steps[_mainStepID]._goto = id;
    }
 
    // 处理退出语句
    void Script::_process_Exit() {
        _state._steps[_mainStepID].commands.push_back("exit");
    }
};