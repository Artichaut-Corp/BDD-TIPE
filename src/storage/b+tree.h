#include "common.h"
#include "types.h"

#include <cassert>
#include <cstdio>
#include <unistd.h>
#include <vector>

#ifndef B_TREE_H
#define B_TREE_H

namespace Database::Storing {

template <DbUInt8 N>
class BPlusTree {

    /*
    struct IndexPayload {
        // Used only for
        const DbKey m_Key;
        const ColumnData m_Data;
    };

      struct DataPayload {
          const T m_Data;
          const DbInt8 m_DataSize;
      };
  */

public:
    const int m_Fd;

    DbUInt64 m_NextOffset;

    const DbUInt8 m_ElementSize;

    const DbUInt8 m_LeafSize;

    const DbUInt8 m_InnerSize;

    BPlusTree(int fd, DbUInt64 next_off, DbUInt8 e_size, DbUInt8 leaf_size, DbUInt8 inner_size)
        : m_Fd(fd)
        , m_NextOffset(next_off)
        , m_ElementSize(e_size)
        , m_LeafSize(leaf_size)
        , m_InnerSize(inner_size)
    {
    }

    struct Node {
        // Not stored because deduced from column type
        DbUInt8 m_ElementSize;

        DbInt64 m_Offset = 0;

        DbBool m_IsLeaf = false;

        DbUInt8 m_UsedCellNumber = 0;

        DbUInt8 m_ChildNumber = 0;

        DbUInt64 m_LeftSibling = 0;
        DbUInt64 m_RightSibling = 0;

        std::vector<ColumnData> m_Keys; // Size N - 1 at most
        std::vector<DbUInt64> m_Childs; // N at most

        bool IsFull() const { return m_UsedCellNumber > N - 2; }

        void Split(const int fd,

            DbUInt64* next_offset,

            const DbUInt8 leaf_size,

            const DbUInt8 inner_size)
        {
            Node* left = new Node();
            Node* right = new Node();

            left->m_ElementSize = m_ElementSize;
            right->m_ElementSize = m_ElementSize;

            if (m_IsLeaf) {
                left->m_IsLeaf = true;
                right->m_IsLeaf = true;

                left->m_Offset = *next_offset;
                right->m_Offset = *next_offset + leaf_size;

                *next_offset += 2 * leaf_size;

                left->m_LeftSibling = m_LeftSibling;
                left->m_RightSibling = right->m_Offset;

                right->m_LeftSibling = left->m_Offset;
                right->m_RightSibling = m_RightSibling;
            } else {
                left->m_IsLeaf = false;
                right->m_IsLeaf = false;

                left->m_Offset = *next_offset;
                right->m_Offset = *next_offset + inner_size;

                *next_offset += 2 * inner_size;
            }

            int mid = (TREE_ORDER + 1) / 2;

            ColumnData midKey = m_Keys[mid];

            // left half of the keys copied to left
            for (int i = 0; i < mid; i++) {
                left->m_Keys[i] = m_Keys[i];
                left->m_UsedCellNumber++;

                left->m_Childs[i] = m_Childs[i];
            }

            left->m_Childs[left->m_ChildNumber] = m_Childs[mid];

            // right half copied to right
            if (!m_IsLeaf) {
                mid++; // do not add mid to the right array
            }

            for (int i = mid; i < m_UsedCellNumber; i++) {
                right->m_Keys[i - mid] = m_Keys[i]; // i-mid == index for right node
                right->m_UsedCellNumber++;

                right->m_Childs[i - mid] = m_Childs[i];
            }

            right->m_Childs[m_UsedCellNumber - mid] = m_Childs[m_ChildNumber];

            // only one key in current node now (midkey that was pushed up)

            m_Keys = std::vector<ColumnData>();

            m_Keys[0] = midKey;
            m_UsedCellNumber = 1;

            // 2 children ptrs only to newly formed left and right

            m_Childs = std::vector<DbUInt64>();

            m_Childs[0] = left->m_Offset;
            m_Childs[1] = right->m_Offset;

            if (m_IsLeaf)
                m_IsLeaf = false;

            left->SerializeLeaf(fd);
            right->SerializeLeaf(fd);

            SerializeLeaf(fd);
            // return midKey;
        }

        bool InsertElement(const ColumnData value)
        {

            if (IsFull()) {
                return false;
            }

            if (m_UsedCellNumber == 0) {
                m_Keys.push_back(value);

                m_UsedCellNumber++;

                return true;
            }

            for (int i = 0; i < m_UsedCellNumber; i++) {
                // new value must be inserted before m_Keys[i]
                if (value <= m_Keys[i]) {
                    // shift all values to the rigth to make room
                    for (int x = m_UsedCellNumber; x > i; x--) {
                        m_Keys[x] = m_Keys[x - 1];
                    }

                    // and inserts
                    m_Keys[i] = value;

                    m_UsedCellNumber++;

                    return true;
                }
            }

            //  is greater than all the current stored values, then at the end
            m_Keys[m_UsedCellNumber] = value;

            m_UsedCellNumber++;
            return true;
        }

        void SerializeNode(int fd) const
        {
            assert(!m_IsLeaf);

            assert(m_Offset != 0);

            DbUInt64 offset = m_Offset;

            lseek(fd, offset, SEEK_SET);

            FileInterface::WriteField(fd, m_IsLeaf, &offset, DB_BOOL_SIZE);

            FileInterface::WriteField(fd, m_ChildNumber, &offset, DB_UINT8_SIZE);

            auto childs = std::vector<ColumnData>();

            childs.reserve(m_ChildNumber);

            for (int i = 0; i < m_ChildNumber; i++) {
                childs[i] = m_Childs[i];
            }

            FileInterface::WriteVec(fd, m_ChildNumber, childs, &offset, DB_UINT64_SIZE);

            offset = lseek(fd, (N - m_ChildNumber) * DB_UINT64_SIZE, SEEK_CUR);

            FileInterface::WriteField(fd, m_UsedCellNumber, &offset, DB_UINT8_SIZE);

            FileInterface::WriteVec(fd, m_UsedCellNumber, m_Keys, &offset, m_ElementSize);
        }

        void SerializeLeaf(int fd) const
        {
            assert(m_IsLeaf);

            assert(m_Offset != 0);

            DbUInt64 offset = m_Offset;

            lseek(fd, offset, SEEK_SET);

            FileInterface::WriteField(fd, m_IsLeaf, &offset, DB_BOOL_SIZE);

            FileInterface::WriteField(fd, m_UsedCellNumber, &offset, DB_UINT8_SIZE);

            FileInterface::WriteVec(fd, m_UsedCellNumber, m_Keys, &offset, m_ElementSize);

            offset = lseek(fd, (N - 1 - m_UsedCellNumber) * m_ElementSize, SEEK_CUR);

            FileInterface::WriteField(fd, m_LeftSibling, &offset, DB_UINT64_SIZE);

            FileInterface::WriteField(fd, m_RightSibling, &offset, DB_UINT64_SIZE);
        }

        static Node ReadLeaf(int fd, DbUInt8 e_size, DbInt64 start_offset)
        {
            DbUInt64 offset = start_offset;

            lseek(fd, offset, SEEK_SET);

            DbBool is_leaf;

            FileInterface::ReadField(fd, &is_leaf, &offset, DB_BOOL_SIZE);

            assert(is_leaf);

            DbUInt8 used_cell_num;

            FileInterface::ReadField(fd, &used_cell_num, &offset, DB_UINT8_SIZE);

            std::vector<ColumnData> keys;

            FileInterface::ReadVec(fd, keys, &offset, e_size, used_cell_num);

            offset = lseek(fd, (N - 1 - used_cell_num) * e_size, SEEK_CUR);

            DbUInt64 l_sibling;
            DbUInt64 r_sibling;

            FileInterface::ReadField(fd, &l_sibling, &offset, DB_UINT64_SIZE);
            FileInterface::ReadField(fd, &r_sibling, &offset, DB_UINT64_SIZE);

            return Node(start_offset, used_cell_num, keys, l_sibling, r_sibling, e_size);
        }

        static Node ReadInternalNode(int fd, DbUInt8 e_size, DbInt64 start_offset)
        {
            DbUInt64 offset = start_offset;

            lseek(fd, offset, SEEK_SET);

            DbBool is_leaf;

            FileInterface::ReadField(fd, &is_leaf, &offset, DB_BOOL_SIZE);

            assert(!is_leaf);

            DbUInt8 child_num;

            FileInterface::ReadField(fd, &child_num, &offset, DB_UINT8_SIZE);

            std::vector<DbUInt64> childs;

            FileInterface::ReadVec(fd, childs, &offset, DB_UINT64_SIZE, child_num);

            offset = lseek(fd, (N - child_num) * DB_UINT64_SIZE, SEEK_CUR);
            DbUInt8 used_cell_num;

            FileInterface::ReadField(fd, &used_cell_num, &offset, DB_UINT8_SIZE);

            std::vector<ColumnData> keys;

            FileInterface::ReadVec(fd, keys, &offset, e_size, used_cell_num);

            return Node(start_offset, child_num, childs, used_cell_num, keys, e_size);
        }

        // Constructor for root
        Node(int fd, DbUInt8 e_size, DbUInt64 starts_at)
        {
            // Read child number
            DbUInt64 offset;

            DbBool is_leaf;

            DbUInt8 child_num;

            DbUInt8 used_cell_num;

            std::vector<DbUInt64> childs;

            std::vector<ColumnData> keys;

            DbUInt64 l_sibling;

            DbUInt64 r_sibling;

            FileInterface::ReadField(fd, &is_leaf, &offset, DB_BOOL_SIZE);

            if (!is_leaf) {
                FileInterface::ReadField(fd, &child_num, &offset, DB_UINT8_SIZE);

                FileInterface::ReadField(fd, &used_cell_num, &offset, DB_UINT8_SIZE);

                FileInterface::ReadVec(fd, childs, &offset, DB_UINT64_SIZE, child_num);

                offset = lseek(fd, (N - child_num) * DB_UINT64_SIZE, SEEK_CUR);

                FileInterface::ReadVec(fd, keys, &offset, e_size, used_cell_num);

            } else {

                FileInterface::ReadField(fd, &used_cell_num, &offset, DB_UINT8_SIZE);

                std::vector<ColumnData> keys;

                FileInterface::ReadVec(fd, keys, &offset, e_size, used_cell_num);

                offset = lseek(fd, (N - 1 - used_cell_num) * e_size, SEEK_CUR);

                FileInterface::ReadField(fd, &l_sibling, &offset, DB_UINT64_SIZE);

                FileInterface::ReadField(fd, &r_sibling, &offset, DB_UINT64_SIZE);
            }

            m_ElementSize = e_size;

            m_Offset = starts_at;

            m_IsLeaf = is_leaf;

            m_ChildNumber = child_num;

            m_UsedCellNumber = used_cell_num;

            m_Childs = childs;

            m_Keys = keys;

            m_LeftSibling = l_sibling;

            m_RightSibling = r_sibling;
        }

        Node() = default;

        // Constructor for leaves
        Node(DbUInt64 offset, DbUInt8 used_cell, const std::vector<ColumnData>& keys, DbUInt64 l_sibling, DbUInt64 r_sibling, DbUInt8 e_size)
            : m_IsLeaf(true)
            , m_ElementSize(e_size)
            , m_Offset(offset)
            , m_UsedCellNumber(used_cell)
            , m_Keys(keys)
            , m_LeftSibling(l_sibling)
            , m_RightSibling(r_sibling)
        {
        }

        // Constructor for inner leaves
        Node(DbUInt64 offset, DbUInt8 child_num, const std::vector<DbUInt64>& childs, DbUInt8 used_cell, const std::vector<ColumnData>& keys, DbUInt8 e_size)
            : m_IsLeaf(false)
            , m_ElementSize(e_size)
            , m_Offset(offset)
            , m_ChildNumber(child_num)
            , m_Childs(childs)
            , m_UsedCellNumber(used_cell)
            , m_Keys(keys)
        {
        }
    };

    void WriteRoot(DbUInt64 root_offset)
    {
        BPlusTree<N>::Node root = Node(root_offset, 0, std::vector<ColumnData>(N - 1), root_offset, root_offset, m_ElementSize);

        root.SerializeLeaf(m_Fd);

        m_NextOffset += m_InnerSize;
    }

    BPlusTree<N>::Node FindRoot(DbUInt64 offset)
    {

        lseek(m_Fd, offset, SEEK_SET);

        return BPlusTree<N>::Node(m_Fd, m_ElementSize, offset);
    }

    BPlusTree<N>::Node SearchLeaf(BPlusTree<N>::Node n, const ColumnData k)
    {
        if (n.m_IsLeaf)
            return n;

        for (int i = 0; i < n.m_ChildNumber; i++) {

            if (k < n.m_Keys[i]) {

                const auto next = BPlusTree<N>::Node::ReadInternalNode(m_Fd, m_ElementSize, n.m_Childs[i]);

                return SearchLeaf(next, k);
            }
        }

        // Maybe some cases where it is needed to look in one more at the right
        // return SearchLeaf(k, n.m_Childs[])
    }

    bool Search(const BPlusTree<N>::Node root, const ColumnData k)
    {
        const BPlusTree<N>::Node leaf = SearchLeaf(root, k);

        for (int i = 0; i < leaf.m_Keys.size(); i++) {
            if (leaf.m_Keys[i] == k)
                return true;
        }

        return false;
    }

    void Insert(const BPlusTree<N>::Node root, const ColumnData value)
    {
        BPlusTree<N>::Node leaf = SearchLeaf(root, value);

        if (leaf.InsertElement(value)) {
            // Replace and reorder the values with the new one and serialize back

            leaf.SerializeLeaf(m_Fd);

            return;
        }

        leaf.Split(m_Fd, &m_NextOffset, m_LeafSize, m_InnerSize);
    }
};

}

#endif //! BPL
