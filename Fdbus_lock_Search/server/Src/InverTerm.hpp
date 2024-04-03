#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>
#include <sys/stat.h>
using namespace std;
#ifndef _INVERTERM_HPP_
#define _INVERTERM_HPP_
// 倒排项创建
class InvertTerm {
    public:
        // 第一次创建,传入文件的名称和位置
        InvertTerm(string docid, int location)
            : docid_(docid)
        {
            freqs_ = 1;
            locations_.push_back(location);
        }
        //给出判断的标准的仿函数
        bool operator==(const InvertTerm& term) const {
            return docid_ == term.docid_;
        }
        // 给出排序的依据
        bool operator<(const InvertTerm& term) const {
            return docid_ < term.docid_;
        }


        string docid_;            // 单词所在的文档的名称
        int freqs_;               // 单词在文档中的出现次数
        // 一个单词在文档中会出现在多个位置所以使用vector来进行存储
        vector<int> locations_;   // 单词在文档中的出现位置
};

class InvertList{
    public:
        // 传入单词所在的文章，单词在这个文章出现的位置
        void add_term(string docid, int location){
            // 遍历倒排项列表中的每个倒排项
            for (auto& term : term_list_) {
                // 如果这个单词所在的文章已经出现在这个倒排项列表中
                if (term.docid_ == docid) {
                    // 单词在这个文章中的频次++
                    term.freqs_++;
                    // 更新单词在这个文章中的位置
                    term.locations_.emplace_back(location);
                    return;
                }
            }
            // 这个单词所在的这个文章没有被记录过,需要创建新的倒排项去存储这个文章的信息
            InvertTerm Item(docid,location);
            term_list_.emplace_back(Item);
        }
        // 获取倒排列表的内容
        const vector<InvertTerm>& get_invert_list() const {
            return term_list_;
        }

    private:
        // 倒排项列表，存储着与这个单词关联的倒排项
        vector<InvertTerm> term_list_;
};
// 倒排索引实现
// 根据传入的目录，遍历目录下的所有.txt文件
class InvertIndex {
    public:
        void set_search_path(string path);
        // 根据传入的目录/文件路径从而获取文件
        // 文件获取操作
        void get_all_file(const char* Path);
        void create_invert_index();
        void ShowFile_List();
        // 返回的是查询到的文章的fd,server这边读完了文章将把fd关闭就行
        string Query(string &Data);
    private:
        // 存储所有的文件路径
        vector<string> file_list_;
        // Key是string，value是相应的倒排索引列表
        // 一个string就是一个单词，InvertList是这个单词对应的倒排项列表
        unordered_map<string, InvertList> invert_map_;
};
#endif 