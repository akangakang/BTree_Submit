
#include "utility.h"
#include <functional>
#include <cstddef>
#include "exception.h"
#include <iostream>
#include <fstream>
#include <cstring>

const int M=3;   //一个中间结点有 M 个孩子 有M-1 个key
const int L=2;
const int Mmin=M/2;
const int Lmin=L/2;

const  int MAX_FILENAME_LEN=30;

namespace sjtu {
    template <class Key, class Value, class Compare = std::less<Key> >
    class BTree {
    public:
        typedef pair<const Key, Value> value_type;

    private:
        // Your private members go here


        struct filenamestring {
            char *str;

            filenamestring() {str=new char[MAX_FILENAME_LEN];strcpy(str,"mybpt.txt");}
            ~filenamestring(){ if(!str) delete str;}


        };

        struct internal_node{

            int offset;   //自己的偏移量
            int father;   //爸爸的偏移量
            int numOfkey; //已存的key的个数
            int type;     //type=0 则它的儿子是internal_node

            int ch[M+1];  //设计每个中间结点存M个孩子  数组开M+1 大小
            Key key[M];   //设计每个中间结点可以放M-1个key  但数组要开M个 因为要是已有M-1个key插入一个key 则先存M个再分裂


            internal_node()
            {
                offset=0;
                father=0;
                numOfkey=0;
                type=0;
                for(int i=0;i<=M;i++) ch[i]=0;

            }

        };

        struct leaf_node{

            int offset;
            int father;
            int prev,next;
            int numOfpair;

            //value_type data[L+1];   //每个叶子结点放L个数据 数组开L+1 给插入留余量   我去pair只能读不能写？？？！！！！
            Key k[L+1];
            Value v[L+1];

            leaf_node(){
                offset=0;
                father=0;
                numOfpair=0;
                prev=0;
                next=0;
            }

        };

        struct basic_info{
            int leaf_node_head;   //第一个叶子结点的位置
            int leaf_node_tail;   //最后一个叶子结点的位置
            int root;   //根结点的位置   一开始应该在base的后面
            int size;
            int eof;    //文件末尾

            basic_info()
            {
                leaf_node_head=0;
                leaf_node_tail=0;
                root=0;
                size=0;
                eof=0;
            }
        };

    private:
        FILE *fp;
        bool fp_is_open;
        basic_info base;
        bool file_already_exists=0;
        filenamestring fp_name;




    public:



        //===============================迭代器==================================//
        class const_iterator;
        class iterator {

            friend class BTree;

        private:
            // Your private members go here
            int leaf_offset;
            int pair_pos;
            BTree *bpt;

        public:
            bool modify(const Value& value){

            }
            iterator() {
                // TODO Default Constructor
                bpt=NULL;
                leaf_offset=0;
                pair_pos=0;
            }
            iterator(const iterator& other) {
                // TODO Copy Constructor
                bpt=other.bpt;
                leaf_offset=other.leaf_offset;
                pair_pos=other.pair_pos;
            }
            // Return a new iterator which points to the n-next elements
            iterator operator++(int) {
                // Todo iterator++

            }
            iterator& operator++() {
                // Todo ++iterator
            }
            iterator operator--(int) {
                // Todo iterator--
            }
            iterator& operator--() {
                // Todo --iterator
            }
            // Overloaded of operator '==' and '!='
            // Check whether the iterators are same
            bool operator==(const iterator& rhs) const {
                // Todo operator ==
            }
            bool operator==(const const_iterator& rhs) const {
                // Todo operator ==
            }
            bool operator!=(const iterator& rhs) const {
                // Todo operator !=
            }
            bool operator!=(const const_iterator& rhs) const {
                // Todo operator !=
            }
        };
        class const_iterator {
            friend class BTree;
            // it should has similar member method as iterator.
            //  and it should be able to construct from an iterator.
        private:
            // Your private members go here
            int leaf_offset;
            int pair_pos;
            BTree *bpt;
        public:
            const_iterator() {
                // TODO
                bpt=NULL;
                leaf_offset=0;
                pair_pos=0;
            }
            const_iterator(const const_iterator& other) {
                // TODO
                bpt=other.bpt;
                leaf_offset=other.leaf_offset;
                pair_pos=other.pair_pos;
            }
            const_iterator(const iterator& other) {
                // TODO
                bpt=other.bpt;
                leaf_offset=other.leaf_offset;
                pair_pos=other.pair_pos;
            }
            // And other methods in iterator, please fill by yourself.
        };


        //===================================文件操作=====================================//

        void openfile(){
            file_already_exists=1;
            if(fp_is_open==0)
            {
               // fp= fopen(fp_name.str,"rb+");
                //打开成功fopen返回指针地址，否则返回NULL
                //然后检查一下文件是否打开成功  没打开成功说明文件不存在嗷
                //if(fp==NULL)
               // {
                    //w：只能向文件写数据，若指定的文件不存在则创建它，如果存在则先删除它再重建一个新文件
                    fp = fopen(fp_name.str,"w");
                    //现在有文件啦  再以rw+打开  但是这个文件里没有bpt的基本信息嗷  待会用build_a_tree往里面写
                    fclose(fp);
                    fp =fopen(fp_name.str,"rb+");

                //}


                    //说明刚刚打开成功啦 已经有 bpt的信息啦 现在要读basic_info进内存
             //   else{
               //     ReadFile(&base,0,1, sizeof(basic_info));
                //}

                fp_is_open=1;
            }
        }

        void ReadFile(void *place,int offset,int count,int size)
        {
            fseek(fp,offset,SEEK_SET);
            fread(place,size,count,fp);
        }

        void WriteFile(void *place, int offset, int count ,int size)
        {
            fseek(fp,offset,SEEK_SET);
            fwrite(place,size,count,fp);

        }


        //=======================================构造 析构======================================//
        void build_a_tree()
        {
            base.size=0;
            internal_node root;
            leaf_node leaf_left;
            leaf_node leaf_right;

            //确定各个offset
            //根排在base后面  左叶子在根后面  右叶子在左叶子后面
            root.offset= sizeof(basic_info);
            base.root=root.offset;
            //第一个叶子结点排在根后面
            leaf_left.offset=sizeof(basic_info)+ sizeof(internal_node);
            leaf_right.offset=leaf_left.offset+ sizeof(leaf_node);
            base.leaf_node_head=leaf_left.offset;
            base.leaf_node_tail=leaf_right.offset;

            //维护eof
            base.eof=sizeof(basic_info)+ sizeof(internal_node)+sizeof(leaf_node)*2;

            //设置 根 和 叶子结点的 参数
            root.father=0;
            root.numOfkey=0;
            root.type=1;                //它指向了叶子结点
            root.ch[0]=leaf_left.offset;
            root.ch[1]=leaf_right.offset;

            leaf_left.father=root.offset;
            leaf_right.father=root.offset;
            leaf_right.prev=leaf_left.offset;
            leaf_left.prev=0;
            leaf_left.next=leaf_right.offset;
            leaf_right.next=0;      //最后一个叶子的next是0？？可以吗？？

            leaf_left.numOfpair=leaf_right.numOfpair=0;

            //然后吧这些东西写到文件里面去！！
            WriteFile(&base,0,1,sizeof(basic_info));
            WriteFile(&root,root.offset,1,sizeof(internal_node));
            WriteFile(&leaf_left,leaf_left.offset,1,sizeof(leaf_node));
            WriteFile(&leaf_right,leaf_right.offset,1,sizeof(leaf_node));

            /*
             * 注意一开始构造之后root里面是没有key的
             * 但是根有两个孩子 孩子里面也没有数据
             */

        }
        // Default Constructor and Copy Constructor
        BTree() {
            // Todo Default

            fp=NULL;
            openfile();

            //如果刚刚那个文件是刚刚w创建的就要build_a_tree
           // if(file_already_exists==0)
                build_a_tree();

        }
        BTree(const BTree& other) {
            // Todo Copy

        }
        BTree& operator=(const BTree& other) {
            // Todo Assignment
        }
        ~BTree() {
            // Todo Destructor

            if(fp_is_open==1)
            {
                fclose(fp);
                fp_is_open=0;
            }

        }

        //====================================================功能==============================================//

        //========================对叶子结点的操作===========================//
        int find_in_leaf(Key key,int offset)
        {
            internal_node p;

            ReadFile(&p,offset, 1, sizeof(internal_node));

            if(p.type==1)   //说明它的下一个是叶子结点啦
            {

                int pos=0;
                for(pos=0;pos<p.numOfkey;pos++)
                {
                    if(key<p.key[pos])  break;
                }

                //注意孩子数比结点数多一个 pos++ 跳出循环后 也有孩子的
                return p.ch[pos];
            }

            else{
                int pos=0;
                for(pos=0;pos<p.numOfkey;pos++)
                {
                    if(key<p.key[pos]) break;
                    if(key==p.key[pos])    //考虑等号？？？
                    {
                        pos++;
                        break;
                    }
                }

                return find_in_leaf(key,p.ch[pos]);
            }

        }

        pair<iterator,OperationResult> insert_leaf(leaf_node &leaf,Key key,Value value)
        {
            iterator ret;
            int pos=0;

            for(pos=0;pos<leaf.numOfpair;pos++)
            {
                //if(key==leaf.k[pos]) return pair<iterator,OperationResult >(iterator(NULL),Fail);
                if(key<leaf.k[pos]) break;
            }

            //刚开始插入数据的时候 第一次在根的左孩子里插入时 根的左孩子还没有数据
            if(leaf.numOfpair==0)
            {
                leaf.k[0]=key;
                leaf.v[0]=value;

                //维护size!!!!!
                leaf.numOfpair=1;
                ++base.size;

                //设置迭代器
                ret.bpt=this;
                ret.pair_pos=pos;
                ret.leaf_offset=leaf.offset;

                WriteFile(&leaf,leaf.offset,1,sizeof(leaf_node));

                return pair<iterator,OperationResult > (ret,Success);
            }
            //每个数据后移
            for(int i=leaf.numOfpair-1;i>=pos;--i)
            {
                leaf.k[i+1]=leaf.k[i];
                leaf.v[i+1]=leaf.v[i];
            }

            //插入
            leaf.k[pos]=key;
            leaf.v[pos]=value;

            //维护size
            ++leaf.numOfpair;
            ++base.size;

            //设置迭代器
            ret.bpt=this;
            ret.pair_pos=pos;
            ret.leaf_offset=leaf.offset;

            //std::cout<<"successful inser leaf "<<key<<'\n';
            //std::cout<<"now the numofpair in this leaf is "<<leaf.numOfpair<<'\n';

            //把这个new_node写进文件覆盖掉原node  考虑分裂
            if(leaf.numOfpair<=L)
                WriteFile(&leaf,leaf.offset,1,sizeof(leaf_node));
            else
                split_leaf(leaf,ret,key);

            return pair<iterator,OperationResult > (ret,Success);




        }


        void split_leaf (leaf_node &leaf, iterator & it, Key &key)
        {
            leaf_node new_leaf;

            //维护numOfpair
            new_leaf.numOfpair=leaf.numOfpair-leaf.numOfpair/2;
            leaf.numOfpair/=2;

            //维护new_leaf的offset
            new_leaf.offset=base.eof;

            for(int i=0;i<new_leaf.numOfpair;i++)
            {
                new_leaf.k[i]=leaf.k[leaf.numOfpair+i];
                new_leaf.v[i]=leaf.v[leaf.numOfpair+i];


                //维护iterator  如果新插入数据被分到new_leaf
                if(new_leaf.k[i]==key)
                {
                    it.leaf_offset=new_leaf.offset;
                    it.pair_pos=i;
                }
            }

            //维护叶子结点 顺序  注意读入leaf.next 修改它的prev
            new_leaf.next=leaf.next;
            new_leaf.prev=leaf.offset;

            if(leaf.next!=0)
            {
                leaf_node leaf_next;
                ReadFile(&leaf_next,leaf.next,1, sizeof(leaf_node));
                leaf_next.prev=new_leaf.offset;
                WriteFile(&leaf_next,leaf_next.offset,1,sizeof(leaf_node));
            }
            leaf.next=new_leaf.offset;

            //维护base
            if(base.leaf_node_tail==leaf.offset)  base.leaf_node_tail=new_leaf.offset;
            base.eof+= sizeof(leaf_node);

            //维护爸爸
            new_leaf.father=leaf.father;

            WriteFile(&leaf,leaf.offset,1, sizeof(leaf_node));
            WriteFile(&new_leaf,new_leaf.offset,1, sizeof(leaf_node));
            WriteFile(&base,0,1, sizeof(basic_info));

            /*std::cout<<"split_leaf"<<'\n';
            std::cout<<"cout old leaf key"<<'\n';
            for(int i=0;i<leaf.numOfpair;i++)
            {
                std::cout<<leaf.k[i]<<" ";
            }
            std::cout<<'\n';
            std::cout<<"cout new leaf key"<<'\n';
            for(int i=0;i<new_leaf.numOfpair;i++)
            {
                std::cout<<new_leaf.k[i]<<" ";
            }
            std::cout<<'\n';

             */
            //维护爸爸

            internal_node father;
            ReadFile(&father,leaf.father,1,sizeof(internal_node));
            insert_node(father,new_leaf.k[0],new_leaf.offset);


        }

        //==============================对中间结点的操作===================================//
        void insert_node(internal_node & cur,Key key,int new_leaf_offset)
        {
            /*std::cout<<"before insert_node, the node contains key "<<'\n';
            for(int i=0;i<cur.numOfkey;i++)
            {
                std::cout<<cur.key[i]<<" ";
            }
            std::cout<<'\n';
             */

            int pos=0;
            for(pos=0;pos<cur.numOfkey;++pos)
            {
                if(key<cur.key[pos]) break;
            }

            for(int i=cur.numOfkey-1;i>=pos;i--)
            {
                cur.key[i+1]=cur.key[i];
            }

            for(int i=cur.numOfkey;i>=pos+1;i--)
            {
                cur.ch[i+1]=cur.ch[i];
            }

            cur.key[pos]=key;   //把新叶子结点的第一个key写进爸爸的key  放在pos(下标)的位置
            cur.ch[pos+1]=new_leaf_offset; //新孩子的offset写在pos+1的位置！！！

            //维护numOfkey
            ++cur.numOfkey;

            /*std::cout<<"after insert_node the node contains key"<<'\n';
            for(int i=0;i<cur.numOfkey;i++)
            {
                std::cout<<cur.key[i]<<'\n';
            }*/

            // 写回去  考虑分裂node
            if(cur.numOfkey<=M-1)
                WriteFile(&cur,cur.offset,1,sizeof(internal_node));
            else
                split_internal(cur);



        }

        void split_internal(internal_node & node)
        {
            /*std::cout<<"before spilt_internal the node contains key ";
            for(int i=0;i<node.numOfkey;i++)
            {
                std::cout<<node.key[i]<<" ";
            }
            std::cout<<'\n';
             */

            internal_node new_node;

            //维护new_node的信息

            /*
             * 现在node的key的个数应该是m个
             * 我希望分裂后node 有M/2个key new_node有 M-M/2-1个key
             * 加起来有M-1个 中间一个key应该上移给node的爸爸
             * 这样node 有M/2+1个child  new_node有M-M/2个child
             */
            new_node.numOfkey=node.numOfkey-node.numOfkey/2-1;
            node.numOfkey/=2;


            new_node.offset=base.eof;
            base.eof+= sizeof(internal_node);
           /* std::cout<<"new node's numofkey"<<"'\n";
            std::cout<<new_node.numOfkey<<'\n';

            std::cout<<"node's numofkey"<<"'\n";
            std::cout<<node.numOfkey<<'\n';
            */

            //更新new_node的数据
            for(int i=0;i<new_node.numOfkey;i++)
            {
                new_node.key[i]=node.key[i+node.numOfkey+1];  //从numOfkey+1开始抄  numOfkey号key应该写到爸爸里
            }

            //std::cout<<"there"<<'\n';
            for(int i=0;i<=new_node.numOfkey;i++)
            {
                new_node.ch[i]=node.ch[i+node.numOfkey+1];   //从numOfkey+1（下标）开始抄 numOfkey号（下标）放在node的最后一个
            }

           /* std::cout<<"new node's key"<<'\n';
            for(int i=0;i<new_node.numOfkey;i++)
            {
                std::cout<<new_node.key[i]<<" ";
            }
            std::cout<<'\n';
            */

            new_node.type=node.type;


            //更新新结点的孩子们的father


            for(int i=0;i<=new_node.numOfkey;i++)   //这里用的等号？？？应该吧 因为孩子数比key数多一个
            {
                if(new_node.type==1)
                {
                    leaf_node leaf;
                    ReadFile(&leaf,new_node.ch[i],1,sizeof(leaf_node));
                    leaf.father=new_node.offset;
                    WriteFile(&leaf,leaf.offset,1,sizeof(leaf_node));
                }
                else{
                    internal_node internal;
                    ReadFile(&internal,new_node.ch[i],1,sizeof(internal_node));
                    internal.father=new_node.offset;
                    WriteFile(&internal,internal.offset,1,sizeof(internal_node));
                }

            }





            //把中间那个插到爸爸里  考虑爸爸是root
            if(node.offset == base.root)
            {
                //std::cout<<"new root will be build"<<'\n';
                //新建一个根
                internal_node new_root;
                new_root.father=0;
                new_root.type=0;
                new_root.offset=base.eof;    //新的root在文件尾  更新base.root!!
                base.eof+= sizeof(internal_node);
                new_root.numOfkey=1;


                //设置新根的key ch
                new_root.key[0]=node.key[node.numOfkey];
                new_root.ch[0]=node.offset;
                new_root.ch[1]=new_node.offset;

                //更新node new_node的爸爸
                node.father=new_root.offset;
                new_node.father=new_root.offset;

                //更新base的root;
                base.root=new_root.offset;

               /*std::cout<<"new root's key"<<'\n';
                for(int i=0;i<new_root.numOfkey;i++)
                {
                    std::cout<<new_root.key[i]<<' ';
                }
                std::cout<<'\n';

                std::cout<<"node's key"<<'\n';
                for(int i=0;i<node.numOfkey;i++)
                {
                    std::cout<<node.key[i]<<" ";
                }
                std::cout<<'\n';

                std::cout<<"new node's key"<<'\n';
                for(int i=0;i<new_node.numOfkey;i++)
                {
                    std::cout<<new_node.key[i]<<" ";
                }
                std::cout<<'\n';
                */


                //写进文件
                WriteFile(&base,0,1,sizeof(basic_info));
                WriteFile(&node,node.offset,1,sizeof(internal_node));
                WriteFile(&new_node,new_node.offset,1, sizeof(internal_node));
                WriteFile(&new_root,new_root.offset,1,sizeof(internal_node));


            }
            else
            {
                new_node.father=node.father;
                /*std::cout<<"node's key"<<'\n';
                for(int i=0;i<node.numOfkey;i++)
                {
                    std::cout<<node.key[i]<<" ";
                }
                std::cout<<'\n';

                std::cout<<"new node's key"<<'\n';
                for(int i=0;i<new_node.numOfkey;i++)
                {
                    std::cout<<new_node.key[i]<<" ";
                }
                std::cout<<'\n';
                 */
                WriteFile(&base,0,1,sizeof(basic_info));
                WriteFile(&node,node.offset,1,sizeof(internal_node));
                WriteFile(&new_node,new_node.offset,1, sizeof(internal_node));

                internal_node father;
                ReadFile(&father,node.father,1, sizeof(internal_node));
                insert_node(father,node.key[node.numOfkey],new_node.offset);
            }



        }


        // Insert: Insert certain Key-Value into the database
        // Return a pair, the first of the pair is the iterator point to the new
        // element, the second of the pair is Success if it is successfully inserted
        pair<iterator, OperationResult> insert(const Key& key, const Value& value) {

            /*
             * 总体思路：先找到leaf_node插入（分类）
             *          再更新（分类）
             */

            //从0开始插入的还没写！！！！！！！！
            //现在开始写了
            //经过这轮插入后右孩子有了数据 但左孩子还是空的
            if(base.size==0)
            {
                internal_node root;
                leaf_node leaf_right;

                //手动把key放进root (以后root里面有key了应该可以正常插入了)
                //数据应该放在右孩子里（右孩子放大于等于key的）
                ReadFile(&root,base.root,1, sizeof(internal_node));
                ReadFile(&leaf_right,base.leaf_node_tail,1, sizeof(leaf_right));

                root.key[0]=key;
                leaf_right.k[0]=key;
                leaf_right.v[0]=value;

                //维护size !!! 总是忘！！！
                base.size=1;
                root.numOfkey=1;
                leaf_right.numOfpair=1;

                //设置返回值
                iterator p;
                p.bpt=this;
                p.leaf_offset=leaf_right.offset;
                p.pair_pos=0;


                //写回文件
                WriteFile(&root,base.root,1, sizeof(internal_node));
                WriteFile(&leaf_right,base.leaf_node_tail,1,sizeof(leaf_node));

                return pair<iterator,OperationResult >(p,Success);

            }

            //找到在哪个叶子结点
            int cur_leaf_node_offset=find_in_leaf(key,base.root);

            //在该叶子结点里面插入key value
            //先往new_node里面写 然后在write回去 覆盖掉
            leaf_node new_node;

            //<????>
            //先把文件里的数据写进new_node
            ReadFile(&new_node,cur_leaf_node_offset,1, sizeof(leaf_node));
            pair<iterator,OperationResult > ret =insert_leaf(new_node,key,value);
            return ret;

        }
        // Erase: Erase the Key-Value
        // Return Success if it is successfully erased
        // Return Fail if the key doesn't exist in the database
        OperationResult erase(const Key& key) {
            // TODO erase function
            return Fail;  // If you can't finish erase part, just remaining here.
        }
        // Return a iterator to the beginning
        iterator begin() {}
        const_iterator cbegin() const {}
        // Return a iterator to the end(the next element after the last)
        iterator end() {}
        const_iterator cend() const {}
        // Check whether this BTree is empty
        bool empty() const {}
        // Return the number of <K,V> pairs
        size_t size() const {}
        // Clear the BTree
        void clear() {}
        // Return the value refer to the Key(key)
        Value at(const Key& key){

            int leaf_pos = find_in_leaf(key,base.root);
            leaf_node leaf;
            ReadFile(&leaf,leaf_pos,1, sizeof(leaf_node));
            for(int i=0;i<leaf.numOfpair;i++)
            {
                if(leaf.k[i]==key)
                {
                    return leaf.v[i];
                }
            }
            /*iterator it=find(key);
            leaf_node leaf;
            ReadFile(&leaf,it.leaf_offset,1, sizeof(leaf_node));
            return leaf.v[it.pair_pos];*/


        }
        /**
         * Returns the number of elements with key
         *   that compares equivalent to the specified argument,
         * The default method of check the equivalence is !(a < b || b > a)
         */
        size_t count(const Key& key) const {}
        /**
         * Finds an element with key equivalent to key.
         * key value of the element to search for.
         * Iterator to an element with key equivalent to key.
         *   If no such element is found, past-the-end (see end()) iterator is
         * returned.
         */
        iterator find(const Key& key) {
            int leaf_pos = find_in_leaf(key,base.root);
            leaf_node leaf;
            ReadFile(&leaf,leaf_pos,1, sizeof(leaf_node));
            for(int i=0;i<leaf.numOfpair;i++)
            {
                if(leaf.k[i]==key)
                {
                    iterator ret;
                    ret.bpt=this;
                    ret.leaf_offset=leaf_pos;
                    ret.pair_pos=i;
                    return ret;
                }
            }
        }
        const_iterator find(const Key& key) const {
            int leaf_pos = find_in_leaf(key,base.root);
            leaf_node leaf;
            ReadFile(&leaf,leaf_pos,1, sizeof(leaf_node));
            for(int i=0;i<leaf.numOfpair;i++)
            {
                if(leaf.k[i]==key)
                {
                    iterator ret;
                    ret.bpt=this;
                    ret.leaf_offset=leaf_pos;
                    ret.pair_pos=i;
                    return ret;
                }
            }
        }
    };
}  // namespace sjtu

