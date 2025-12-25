#include "b+tree.h"

#include <cstdio>
#include <unistd.h>

namespace Database::Storing {

template <DbUInt8 N>
bool BPlusTree<N>::Node::IsFull() const { return m_UsedCellNumber >= N - 2; }

template <DbUInt8 N>
void BPlusTree<N>::Node::InsertElement(const ColumnData value)
{
    m_UsedCellNumber++;

    // Dichtomie
    if (value > m_Keys[m_UsedCellNumber / 2]) {
    }

    // Shift des valeurs
    //
    //
}

template <DbUInt8 N>
BPlusTree<N>::Node FindRoot(int fd, DbUInt64 offset, const DbUInt8 e_size)
{
    lseek(fd, offset, SEEK_SET);

    return BPlusTree<N>::Node(fd, e_size);
}

template <DbUInt8 N>
BPlusTree<N>::Node BPlusTree<N>::SearchLeaf(int fd, const ColumnData k, BPlusTree<N>::Node n)
{
    if (n.m_IsLeaf)
        return n;

    for (int i = 0; i < n.m_ChildNumber; i++) {

        if (k < n.m_Keys[i]) {

            const auto next = BPlusTree<N>::Node::ReadInternalNode(fd, n.m_Childs[i]);

            return SearchLeaf(fd, k, next);
        }
    }

    // Maybe some cases where it is needed to look in one more at the right
    // return SearchLeaf(k, n.m_Childs[])
}

template <DbUInt8 N>
bool BPlusTree<N>::Search(int fd, const BPlusTree<N>::Node root, ColumnData k)
{

    const BPlusTree<N>::Node leaf = SearchLeaf(fd, k, root);

    for (int i = 0; i < leaf.m_Keys.size(); i++) {
        if (leaf.m_Keys[i] == k)
            return true;
    }

    return false;
}

template <DbUInt8 N>
void BPlusTree<N>::Insert(int fd, const Node root, const ColumnData value)
{

    const BPlusTree<N>::Node leaf = SearchLeaf(value, root);

    if (!leaf.IsFull()) {
        // Replace and reorder the values with the new one and serialize back

        leaf.InsertElement(value);

        leaf.SerializeLeaf(fd);

        return;
    }
}

}
