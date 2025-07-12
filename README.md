# 2025年嵌入式大赛海思赛道代码开源提交流程



## 1、简介

此仓库主要用于2025年嵌入式大赛学生作品代码开源，欢迎将自己的作品提交到本仓库。相关提交请遵守git的具体操作，并满足各个仓库的具体规范要求。



## 2、仓库目录介绍

* 在2025_hisilicon_embedded_competition目录下，一共有两个文件夹，其中AIOT文件夹是海思赛道 AI端侧智能方向的队伍提交代码的目录，IOT文件夹是海思赛道 星闪物联网方向的队伍提交代码的目录。

![1711003179840](pic/1711003179840.png)

* 参赛队伍请根据自己的比赛方向，进入对应的文件夹中，然后 用不同的文件夹来区分不同的组别，文件命名规则为团队编号_作品名称

![1711003320865](pic/1711003320865.png)

* 进到每个团队的目录下，我们需要将关键代码上传到code 目录，还需要撰写README.md 文档，README.md的主要内容是关于你们作品的大致描述以及对整个code目录中的文件进行介绍。

![1711003361709](pic/1711003361709.png)

## 3、 git简介

* 关于git的历史和原理，笔者找了一个介绍比较好的文章，大家可以参考[git原理](https://gitee.com/link?target=https%3A%2F%2Fzhuanlan.zhihu.com%2Fp%2F66506485) 对于初学者而言，下面的图来理解更直观。 ![img](https://gitee.com/openharmony-sig/contest/raw/master/2022_hisilicon_national_embedded_competition/media/git%E5%9F%BA%E6%9C%AC%E6%B5%81%E7%A8%8B.png)

- 1.fork：指的是从官网仓库中复制一份拷贝到自己的账号仓库下，在这个时间节点下两者的内容一致；后续需要不断的手动完成同步；
- 2.clone:指的是从自己的账号仓库下下载到本地端;
- 3.commit:指的是将克隆的代码，根据需要修改更正某些内容或者增加新内容、删除冗余内容，形成记录。
- 4.push:指的是将自己的修改提交到本人账号仓库下；
- 5.pr:指的是将自己的修改从自己的账号仓库下提交到官方账号仓库下；
- 6.merge:指的是官方账号仓库的commiter接受了你的修改；
- 7.fetch:指的是将官方账号仓库的内容拉取到本地。



## 4、提交作品流程实操

### 4.1、创建gitee账号

- 首先你准备一个自己的手机以及邮箱，为方便后续的操作方便，该手机号码以及邮箱没有和gitee平台发生关联。
- 在[gitee官网](https://gitee.com/)完成注册 ![img](https://gitee.com/openharmony-sig/contest/raw/master/2022_hisilicon_national_embedded_competition/media/gitee%E5%AE%98%E7%BD%91.png)
- 根据自己情况设置信息，图里所示个人空间地址很重要，它就是你的用户名。姓名可以是相同的，但用户名是唯一的。 ![img](https://gitee.com/openharmony-sig/contest/raw/master/2022_hisilicon_national_embedded_competition/media/%E6%B3%A8%E5%86%8C%E4%BF%A1%E6%81%AF%E5%A1%AB%E5%86%99.png)
- **如无意外则创建成功** ![img](https://gitee.com/openharmony-sig/contest/raw/master/2022_hisilicon_national_embedded_competition/media/%E8%B4%A6%E5%8F%B7%E5%88%9B%E5%BB%BA%E6%88%90%E5%8A%9F.png)

### 4.2、gitee账号绑定邮箱

* 在注册账号的时候，有些信息我们是没有补充的，基本信息基本只有电话号和密码，能够满足我们的登录并使用该账号“发言”。那么问题来了，为了让我们后续发言更有说服力，我们需要补充相关的信息。邮箱作为互联网的比较典型的通用信息，需要补充完善



![image-20230703180601641](pic/image-20230703180601641.png)

 

![image-20230703180824689](pic/image-20230703180824689.png)



* 按照图示补充邮箱信息，并设置自己的提交邮箱。为自己后续的打怪升级做准备，人民会记住你的贡献的。

![image-20230703180908605](pic/image-20230703180908605.png)



### 4.3、Fork官方仓库

* 所谓Fork，就是把官方仓库当前时间点内容搬迁到自己账号下面，直接在网页上操作即可完成。如我们把赛事活动仓库Fork到自己账号下面。[活动仓库的官方地址。](https://gitee.com/HiSpark/2025_hisilicon_embedded_competition/tree/master)

![1711003809247](pic/1711003809247.png)

* fork之后，在我们的gitee账号就可以看到这个仓库啦。

![1711003918652](pic/1711003918652.png)



### 4.4、安装GIT客户端工具：gitbash

* Windows 环境下建议大家使用命令行的工具，如果你是MACOS或者Linux,我相信你使用起来会更简单，此处不表，本文仅仅以WINDOWS环境介绍。 可以从[git bash下载地址](https://gitee.com/link?target=https%3A%2F%2Fgit-scm.com%2Fdownload%2Fwin)下载git bash工具并安装。 安装完毕之后，在你的工作目录下右键点击即可出现git bash here。

![image-20230705160602737](pic/image-20230705160602737.png)

![image-20230703203430552](pic/image-20230703203430552.png)

* 点击启动 git bash 之后会进入一个linux终端的界面，这就是我们后续将修改内容从本地上传到

* Gitee上的个人仓库的主要战场了。

![image-20230703203500743](pic/image-20230703203500743.png)

### 4.5、设置git客户端

* 使用SSH公钥可以让你在你的电脑和 Gitee 通讯的时候使用安全连接。 那么怎么获取到我们PC的SSH公钥呢？在桌面右键打开git bash

![image-20230703203430552](pic/image-20230703203430552.png)

* 输入**ssh-keygen.exe** 并回车，再次回车，然后输入y，继续回车两次，这样即可生成个人的SSH公钥保持文件。

![image-20230703203644726](pic/image-20230703203644726.png)

* git无法直接ctrl+c/v实现复制粘贴，但可以鼠标选中ssh公钥保持文件（即Your public key has been saved in 后面的内容）然后右键Copy复制，Paste粘贴实现这个功能。

* 使用cat命令查看生成的id_rsa.pub文件，输入cat （右键Paste粘贴ssh公钥保持文件）回车即可查看具体信息。

![image-20230703203733019](pic/image-20230703203733019.png)

* 从ssh-rsa开始，整段选中然后复制，打开gitee官网在设置里面找到ssh公钥，粘贴确定即可将公钥添加到我们的gitee账号中。

![image-20230703203809351](pic/image-20230703203809351.png)

![image-20230703203840145](pic/image-20230703203840145.png)



### 4.6、配置个人信息

* 我们向gitee个人仓库提交修改内容，需要告知大家这些修改内容是谁发起提交的，不然大家怎么知道是哪位英雄好汉为开源社区出了力。所以为了避免每次都重复输入一些提交信息（个人账号信息），我们需要使用git bash统一配置一下提交信息。
* 首先，先记住自己的个人空间地址，在个人主页的网页链接上可看到，我的用户名就是 “wgm2022”

![1711004003189](pic/1711004003189.png)

* 打开git bash，依次输入以下命令并回车，前两个命令没有反应就证明配置成功。

```
git config --global user.name "xxxx"   （配置用户名，xxxx为账号用户名，即个人空间地址）

git config --global user.email "xxxxxx@xxx"   // 与你的gitee 账号邮箱和你签署DCO 的邮箱保持一致即可  

git config --list         （查看配置情况）
```



![image-20230703204144932](pic/image-20230703204144932.png)

### 4.7、克隆仓库内容到本地

* 到个人账号点击并进入这个**2025_hisilicon_embedded_competition**仓库，点击克隆/下载按钮，复制clone的链接地址。

![1711004092314](pic/1711004092314.png)

![1711004068507](pic/1711004068507.png)

* 在git  bash工具下面使用git clone命令完成clone动作。

```
git clone git clone https://gitee.com/wgm2022/2025_hisilicon_embedded_competition.git --depth=1
```

![1711004982366](pic/1711004982366.png)

* --depth=1意思是只clone当前仓库最新版本，省去一些历史log，避免仓库历史记录过于庞大花费太多clone时间。需要注意的是开发者需要克隆自己账号下的仓库，原则上这个地址构成如下.

```
git clone https://gitee.com/账号名/仓库名.git  --depth=1
```

* clone完毕之后，即可在本地目录下看到这个clone的仓库。补充说明一下，本地目录所在位置是根据git bash的位置决定的，比如你在桌面启动git bash，则clone的仓库会出现在桌面。

![1711005091142](pic/1711005091142.png)

### 4.8、添加或修改文件

* 按照要求增加目录、文件，或者修改部分文件内容。

* 首先在2025_hisilicon_embedded_competition目录下，按照文件命名规则：**团队编号_作品名称** 新建你们团队的个人文件夹

![1711003320865](pic/1711003320865.png)

* 进到每个团队的目录下，你们需要将关键代码上传进来，还需要撰写README.md 文档，README.md的主要内容是关于你们作品的大致描述以及对整个code目录中的文件进行介绍。

* 比如说，我的队伍编号是99999，然后我们的作品是手势识别，那我创建的文件的名字为99999_hand_classify，然后我Taurus开发板 只修改了ai_sample的hand_classify文件夹里面的代码，以及smp目录下的代码。Pegasus开发板只使用并修改了uart_demo里面的代码。因此我提供的主要代码如下图所示。

![image-20230705162657919](pic/image-20230705162657919.png)

* readme.md里面的内容，如下图所示进行描述即可。

![image-20230705163903688](pic/image-20230705163903688.png)

### 4.9、开始提交

#### 1、查看修改变更后的文件

```
cd   2025_hisilicon_embedded_competition

git status
```

![image-20230705164304336](pic/image-20230705164304336.png)

#### 2、将变更文件加入到暂存区

```
git add *
```

![image-20230705164633348](pic/image-20230705164633348.png)

#### 3、将暂存区内容签名并提交到本地

```
git commit -s -m  " add:99999_hand_classify init"
```

![image-20230705164815165](pic/image-20230705164815165.png)

* 最后我们再用git status查看一下，可看到已没有修改变更内容存在了。

![image-20230705164909408](pic/image-20230705164909408.png)

#### 4、推送本地修改到账号仓库

* 现在我们需要将本地仓库的修改内容推送到gitee上的个人仓库，使用git push命令来完成这个动作。origin指的是自己的仓库对应的原始远程服务器地址；master标识的是想要提交的分支

```
git push origin master
```

![image-20230705165054837](pic/image-20230705165054837.png)

### 4.10、开始提交PR到海思仓库

#### 1、检查更新

* 进入我们的账号下面，我们查看这个仓库，发现已经发生了变化

![image-20230705165318270](pic/image-20230705165318270.png)

* 进入到你创建的文件夹目录下，并检查一下我们需要提交的代码和文档是否都在里面。

![image-20230705165549580](pic/image-20230705165549580.png)



#### 2、从个人账号仓库下先海思官方仓库提交PR

* 进入个人账号的该仓库下，点击增加**Pull Request**即可开始提交PR。

![image-20230705165844868](pic/image-20230705165844868.png)

* 在标题框内输入 你们团队编号+你们作品的描述，然后再点击 Pull Request按钮，提交PR。

![image-20230705170048187](pic/image-20230705170048187.png)

* 提交成功之后，等待海思工程师审查。

![image-20230705170115055](pic/image-20230705170115055.png)

* 如果你提交完后，出现下面的提示需要你签署贡献值协议的话，就点击此超链接，按照提示进行操作。

![image-20250704173534873](pic/image-20250704173534873.png)



* 海思工程师审查通过之后，就可以看到如下图所示，会显示已合并，并且审查和测试的状态会显示已完成

![image-20230705172643773](pic/image-20230705172643773.png)

* 进入到海思的[官网代码仓](https://gitee.com/HiSpark/2025_hisilicon_embedded_competition)，然后进入到属于你们队伍的那个文件夹中。
* 此时复制浏览器网址中的内容。

![image-20230705172836323](pic/image-20230705172836323.png)

### 3、复制海思官网gitee代码仓中属于你们队伍提交的代码的网址到组委会的作品上传的位置

![image-20230705173808500](pic/image-20230705173808500.png)





### **注意：**如果我们没有及时合并你的代码到我们代码仓，你可以将自己代码仓的路径复制到组委会的作品上传位置也是可以的。

![image-20230705173808500](pic/image-20230705173808501.png)
