#include "InverTerm.hpp"
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>
#include <iostream>
#include <algorithm>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
void InvertIndex::get_all_file(const char* Path){
    struct stat statbuf;// 获取文件的状态信息，判断是不是目录文件
    if (stat(Path, &statbuf) == -1) {
        std::cerr << "Error getting file information" << std::endl;
        perror("stat");
        exit(1);
        return;
    }
    // 如果是个目录文件 
    if (S_ISDIR(statbuf.st_mode)){
        DIR *dp;
        // 打开目录
        if((dp = opendir(Path)) == NULL) {
            std::cerr << "Error opening directory" << std::endl;
            return;
        }
        struct dirent *entry;
        // 遍历这个目录下的所有文件
        while ((entry = readdir(dp))) {
            std::string file_name = entry->d_name;
            if (file_name == "." || file_name == "..") {
                continue;
            }
            // 遍历当前目录下的这些文件,
            std::string full_path = std::string(Path) + "/" + file_name;
            // 递归调用者Path
            this->get_all_file(full_path.c_str());
        }
        closedir(dp);
    }
    else{
        //直接入队保存这个文件路径
        file_list_.emplace_back(Path);
    }
    return;
}

void InvertIndex::ShowFile_List(){
    for(int i=0;i<this->file_list_.size();i++){
        std::cout<< "File_Path: "<< file_list_[i] << std::endl;
    }
}
//返回文件的路径
//用户输入查询的信息，根据用户输入的查询信息进行查询处理
string InvertIndex::Query(string &Data){
    // 开始将Data进行分词处理
    vector<string> word_list;
    char* word = strtok(const_cast<char*>(Data.c_str()), ",\n ");
    while (word != nullptr) {
        if (strlen(word) > 0) {
            word_list.emplace_back(word);
        }
        word = strtok(nullptr, ",\n ");
    }
    // 如果分词结果为空，直接返回
    // 说明用户此时并没有输入任何的语句进行查询
    if (word_list.empty()) {
        std::cout<<"输入的查询内容为空 查询失败"<<std::endl;
        return "NULL";
    }

    

    //只输入了一个单词
    if(word_list.size() == 1){
        auto it = invert_map_.find(word_list[0]);
        if(it == invert_map_.end()){
            std::cout<<word_list[0]<<"未找到"<<std::endl;
            return "NULL";
        }
        
        // 遍历单词对应的倒排列表
        // 这个倒排列表的每一项是一个倒排列表项
        // 倒排列表项保存了单词对应于这个文章的信息，有出现次数，出现的位置
        int Max=0;
        std::string MaxFilePath;
        //找到单词在哪篇文章出现的频次最大
        for (auto& term : it->second.get_invert_list()) {
            
            //cout << "FilePath: "<<term.docid_ << " freqs:" << term.freqs_ << endl;
            if(term.freqs_ > Max){
                Max = term.freqs_;
                //找的是词出现频率最大的这片文章
                MaxFilePath = term.docid_;
            }
        }

        std::cout<<Max<<"  "<<MaxFilePath<<std::endl;

        //printf("搜索完成了\n");        return MaxFilePath;
    }
    else{
        //输入的是一个句子，要对句子进行分析处理
        //分割完后的句子有多个词，每个词都有单独的倒排列表

        //从一个存储倒排列表的数据结构 invert_map_ 中提取用户输入的词所对应的倒排列表，
        //并计算这些倒排列表的交集。
        vector<InvertList> invert_lists;
        for (int i = 0; i < word_list.size(); i++) {
            // 查找当前词在倒排列表中的映射
            auto iter = invert_map_.find(word_list[i]);
            if (iter != invert_map_.end()) {
                // 如果找到该词的倒排列表，则将其添加到invert_lists中
                invert_lists.emplace_back(iter->second);
            }
        }
        if(invert_lists.empty()){
            printf("搜索失败\n");
            return "NULL";
        }
        //invert_lists每个元素就是一个倒排列表，求这些列表的交集
        //这些倒排列表的交集就是一个个的倒排项

        vector<InvertTerm> common_terms; // 存放倒排列表的交集

        // 以第一个表为基准找到交集
        // 将第一个倒排列表中的元素复制到另一个容器v1中，并对v1进行排序
        vector<InvertTerm> v1(invert_lists[0].get_invert_list().begin(), 
            invert_lists[0].get_invert_list().end()); 
        sort(v1.begin(), v1.end()); // 对v1中的元素进行排序，保证集合有序，以便后续计算交集

        // 遍历剩余的倒排列表，计算它们和第一个倒排列表的交集
        for (int i = 1; i < invert_lists.size(); i++) {
            vector<InvertTerm> v2(invert_lists[i].get_invert_list().begin(), 
                    invert_lists[i].get_invert_list().end());
            sort(v2.begin(), v2.end());

            // 使用 set_intersection 求交集，并将结果存放在 common_terms 中
            set_intersection(v1.begin(), v1.end(), v2.begin(), v2.end(), 
                    back_inserter(common_terms));
            
            // 更新 v1 为当前倒排列表与之前交集的结果，清空 common_terms 准备下一次计算
            v1.swap(common_terms);
            common_terms.clear();
        }
        int maxFreqs = 0;
        std::string docidWithMaxFreqs;

        for (int i = 0; i < v1.size(); i++) {
            if (v1[i].freqs_ > maxFreqs) {
                maxFreqs = v1[i].freqs_;
                docidWithMaxFreqs = v1[i].docid_;
            }
        }
        //int fd = open(docidWithMaxFreqs.c_str(), O_RDONLY);
        return docidWithMaxFreqs;
    }
}

//创建倒排索引
// 根据file_list_中搜索到的文章建立倒排索引
void InvertIndex::create_invert_index(){
    // 遍历文件列表中的每个文件
    for (string file_path : file_list_) {
        //  打开文件
        FILE* fp = fopen(file_path.c_str(), "r");
        if (nullptr == fp) {
            cerr << file_path << "打开失败！" << endl;  // 如果文件打开失败，则输出错误信息并继续下一个文件
            continue;
        }

        int location = 0;  // 记录单词在文件中的位置
        const int LINE_SIZE = 1024;  // 定义一行最大长度
        char line_content[LINE_SIZE] = { 0 };  // 存储读取的一行内容
        // 判断文件是否到了末尾
        while (!feof(fp)){
            vector<string> line_word_list;
            fgets(line_content, LINE_SIZE, fp);
            // 按照空格和,对这一行句子进行切割
            char* word = strtok(line_content, ",.\n;\"'-_?! ");

            while (word != nullptr) {
                // 去除单词前后的空白字符
                if (strlen(word) > 0) {
                    line_word_list.emplace_back(word);  // 将单词添加到单词列表中
                }
                word = strtok(nullptr, ",.\n;\"'-_?! ");  // 继续获取下一个单词
            }

            // 完成了一行的遍历，根据这一行遍历的结果开始建立倒排索引表
            // 一行数据分割完成，开始给word_list里面的单词创建或修改倒排列表
            for (string w : line_word_list) {
                location++;//单词在文章中出现的位置
                // 判断这个单词是否建立了索引
                auto iter = invert_map_.find(w);
                // 这个单词没有建立索引
                if (iter == invert_map_.end()) {
                    // 创建一个索引表
                    InvertList list;
                    // 存放这个单词所在的文章路径和单词的出现位置
                    list.add_term(file_path, location);
                    // 将这对(Key,value)插入map中
                    invert_map_.emplace(w, list);
                } 
                else {
                    // w存在于词典invert_map_中，已经有了倒排列表，需要添加倒排项
                    iter->second.add_term(file_path, location);
                }
            }

        }
        printf("完成了一个文件的索引建立\n");
        /*for (auto it = invert_map_.begin(); it != invert_map_.end(); ++it) {
            std::cout << "Key: " << it->first << std::endl;
            for (auto i = it->second.get_invert_list().begin();
                i != it->second.get_invert_list().end(); i++){
                    std::cout<<"Freqs_"<<i->freqs_<<"  docid_"<<i->docid_<<std::endl;
                }
            
        }*/
    }
    
}

void InvertIndex::set_search_path(string path){
    cout << "搜索文件..." << endl;
    // 传入一个文件路径或者目录来进行文件的获取
    // 如果传入的是一个目录就获取该目录下的所有.txt文件
    // 如果传入的是文件则直接读取内容
    get_all_file(path.c_str());
    cout << "完成！" << endl;

    cout << "开始创建倒排索引";
    create_invert_index();
    cout << "完成！" << endl;
}