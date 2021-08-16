# MyShell

## 准备实现的 feature

* 更加全面的 shell 语法

```
LeafExpression := command [options] [arguement] [input/output redirection] [&]
PipedExpression := expression | expression
SequencedExpression := expression ; expression
AndExpression := expression && expression
OrExpression := expression || expression
# BracedExpression := ( expression )
```

注意：

1. 暂时还没有考虑做优先级，并且所有复合运算均从左往右结合。
2. 暂时不支持 `date & who` 这样的语句，但支持 `date &; who`，这是因为我把 `&` 看作是语句的一部分，而语句和语句之间必须要使用非空的 token （这个 token 只能是 ; | && || 中的一个） 分隔，这样的设计能够简化解释器的实现，毕竟解释器是手写的，而不是基于 yacc 之类的工具生成的。如果语法规则太复杂会有点难写

* tab 补全
* 多种终端的视感
    * 主要是考虑到为了将来扩展方便，就做了两个作为示例
    * 最好支持下 zsh 那样的视感
* 支持历史记录 history 命令
    * 上下键使用历史记录
* 支持环境变量 $N 以及 $?

## 开发计划

* 第一轮迭代：单线程 shell 简单实现
    * 实现解释器
    * 实现 history，支持上下键搜索历史记录
    * 实现 cd clr dir echo exit pwd time umask 以及外部命令
* 第二轮迭代：环境变量、从文件中读取
    * 实现 set shift unset
* 第三轮迭代：多线程 shell 和线程管理
    * 实现 bg exec fg jobs
* 第四轮迭代：管道
    * 
* 第五轮迭代：其他恶心的命令实现
    * 编写文档
    * 实现 help test

## Known Feature

1. 在执行 shell 命令的时候

## Known Issues

1. 会产生大量的僵死进程