#include "b+tree.h"
#include <cstdio>
#include <unistd.h>

namespace Database::Storing {

template <DbInt8 N, DbInt8 S>
bool BPlusTree<N, S>::Node::IsFull() const { return m_UsedCellNumber >= N - 2; }

template <DbInt8 N, DbInt8 S>
BPlusTree<N, S>::Node FindRoot(int fd, DbInt64 offset)
{
    lseek64(fd, offset, SEEK_SET);

    return BPlusTree<N, S>::Node(fd);
}

template <DbInt8 N, DbInt8 S>
BPlusTree<N, S>::Node BPlusTree<N, S>::SearchLeaf(int fd, const ColumnData k, BPlusTree<N>::Node n)
{
    if (n.m_IsLeaf)
        return n;

    for (int i = 0; i < n.m_ChildNumber; i++) {

        BPlusTree<N, S>::Node::ReadInternalNode(fd);

        if (k < n.m_Keys[i]) {
            return SearchLeaf(fd, k, n.m_Childs[i]);
        }
    }

    // Maybe some cases where it is needed to look in one more at the right
    // return SearchLeaf(k, n.m_Childs[])
}

template <DbInt8 N, DbInt8 S>
bool BPlusTree<N, S>::Search(int fd, const Node root, ColumnData k)
{

    const BPlusTree<N, S>::Node leaf = SearchLeaf(fd, k, root);

    for (int i = 0; i < leaf.m_Keys.size(); i++) {
        if (leaf.m_Keys[i] == k)
            return true;
    }

    return false;
}

template <DbInt8 N, DbInt8 S>
void BPlusTree<N, S>::Insert(int fd, const Node root, const ColumnData value)
{

    const BPlusTree<N, S>::Node leaf = SearchLeaf(value, root);

    if (!leaf.IsFull()) {
    }
}

}
