#include "b+tree.h"

namespace Database::Utils {

template <DbInt8 N>
bool BPlusTree<N>::Node::IsFull() const { return m_UsedCellNumber >= N - 2; }

template <DbInt8 N>
bool BPlusTree<N>::Search(DbKey k) const
{

    const BPlusTree<N>::Node leaf = SearchLeaf(k, m_Root);

    for (int i = 0; i < leaf.m_Keys.size(); i++) {
        if (leaf.m_Keys[i] == k)
            return true;
    }

    return false;
}

template <DbInt8 N>
BPlusTree<N>::Node BPlusTree<N>::SearchLeaf(const DbKey k, BPlusTree<N>::Node n) const
{
    if (n.m_IsLeaf)
        return n;

    for (int i = 0; i < n.m_UsedCellNumber; i++) {
        if (k < n.m_Keys[i]) {
            return SearchLeaf(k, n.m_Childs[i]);
        }
    }

    // Maybe some cases where it is needed to look in one more at the right
    // return SearchLeaf(k, n.m_Childs[])
}

template <DbInt8 N>
void BPlusTree<N>::Insert(const DbKey value)
{

    const BPlusTree<N>::Node leaf = SearchLeaf(value, m_Root);

    if (!leaf.IsFull()) {
    }
}

}
