#include "algebrizer_types.h"
#include "common.h"
#include "types.h"

#include <cassert>
#include <cstdio>
#include <stdexcept>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#ifndef B_TREE_H
#define B_TREE_H

namespace Database::Storing {

template <DbUInt8 N>
class BPlusTree {

    struct IndexPayload {
        // Used only for
        DbKey m_Key;
        ColumnData m_Data;

        IndexPayload() = default;

        IndexPayload(DbKey key, ColumnData c)
            : m_Key(key)
            , m_Data(c)
        {
        }

        IndexPayload(int fd, DbOffset* offset, const DbElemType type)
        {
            lseek(fd, *offset, SEEK_SET);

            ssize_t size = Convert::TypeToTypeSize(type);

            ssize_t bytes_read = read(fd, &m_Key, DB_NB_ELEMT_INT);

            switch (type) {
            case DbElemType::DbNull:
                m_Data = 0;
                break;
            case DbElemType::DbBool:
                m_Data = GetValueOfType<DbBool>(fd, size, &bytes_read);
                break;
            case DbElemType::DbInt8:
                m_Data = GetValueOfType<DbInt8>(fd, size, &bytes_read);
                break;
            case DbElemType::DbUInt8:
                m_Data = GetValueOfType<DbUInt8>(fd, size, &bytes_read);
                break;
            case DbElemType::DbInt16:
                m_Data = GetValueOfType<DbInt16>(fd, size, &bytes_read);
                break;
            case DbElemType::DbUInt16:
                m_Data = GetValueOfType<DbUInt16>(fd, size, &bytes_read);
                break;
            case DbElemType::DbInt:
                m_Data = GetValueOfType<DbInt>(fd, size, &bytes_read);
                break;
            case DbElemType::DbUInt:
                m_Data = GetValueOfType<DbUInt>(fd, size, &bytes_read);
                break;
            case DbElemType::DbInt64:
                m_Data = GetValueOfType<DbInt64>(fd, size, &bytes_read);
                break;
            case DbElemType::DbUInt64:
                m_Data = GetValueOfType<DbUInt64>(fd, size, &bytes_read);
                break;
            case DbElemType::DbFloat:
                m_Data = GetValueOfType<DbFloat>(fd, size, &bytes_read);
                break;
            case DbElemType::DbFloat64:
                m_Data = GetValueOfType<DbFloat64>(fd, size, &bytes_read);
                break;
            case DbElemType::DbChar:
                m_Data = GetValueOfType<DbChar>(fd, size, &bytes_read);
                break;
            case DbElemType::DbString:
                m_Data = GetValueOfType<DbString>(fd, size, &bytes_read);
                break;
            }

            *offset += bytes_read;
        }

        template <typename T>
        ColumnData GetValueOfType(int fd, ssize_t e_size, ssize_t* bytes_read)
        {
            T res;

            *bytes_read += read(fd, &res, e_size);

            return res;
        }

        void WriteIndexPayload(int fd, DbOffset* offset, const DbElemType type) const
        {
            lseek(fd, *offset, SEEK_SET);

            ssize_t size = Convert::TypeToTypeSize(type);

            ssize_t bytes_written = write(
                fd, &m_Key, DB_NB_ELEMT_INT);

            bytes_written += write(fd, &m_Data, size);

            if (bytes_written != size + DB_NB_ELEMT_INT) {

                std::cout << "Warning less bytes written than expected\n";
            }

            *offset += bytes_written;
        }
    };

    /*
          struct DataPayload {
              const T m_Data;
              const DbInt8 m_DataSize;
          };
      */

public:
    const int m_Fd;

    DbOffset m_NextOffset;

    const DbElemType m_ElementType;

    const DbUInt8 m_LeafSize;

    const DbUInt8 m_InnerSize;

    BPlusTree(int fd, DbOffset next_off, DbElemType e_type, DbUInt8 leaf_size, DbUInt8 inner_size)
        : m_Fd(fd)
        , m_NextOffset(next_off)
        , m_ElementType(e_type)
        , m_LeafSize(leaf_size)
        , m_InnerSize(inner_size)
    {
    }

    struct Node {
        // Not stored because deduced
        DbElemType m_ElementType;

        DbOffset m_Offset = 0;

        DbBool m_IsLeaf = false;

        DbUInt8 m_UsedCellNumber = 0;

        DbUInt8 m_ChildNumber = 0;

        DbOffset m_LeftSibling = 0;
        DbOffset m_RightSibling = 0;

        std::vector<IndexPayload> m_Keys; // Size N - 1 at most

        std::vector<DbOffset> m_Childs; // N at most

        bool IsFull() const { return m_UsedCellNumber > N - 2; }

        void Split(const int fd,

            DbOffset* next_offset,

            const DbUInt8 leaf_size,

            const DbUInt8 inner_size)
        {
            Node* left = new Node();
            Node* right = new Node();

            left->m_ElementType = m_ElementType;
            right->m_ElementType = m_ElementType;

            left->m_Keys.resize(m_UsedCellNumber / 2);
            right->m_Keys.resize(m_UsedCellNumber / 2 + 1);

            left->m_Childs.resize(m_ChildNumber / 2);
            right->m_Childs.resize(m_ChildNumber / 2 + 1);

            if (m_IsLeaf) {
                // free the space needed for the node about to become inner
                *next_offset = *next_offset - leaf_size + inner_size;

                left->m_IsLeaf = true;
                right->m_IsLeaf = true;

                left->m_Offset = *next_offset;
                right->m_Offset = *next_offset + leaf_size;

                *next_offset += 2 * leaf_size;

                left->m_LeftSibling = left->m_Offset;
                left->m_RightSibling = right->m_Offset;

                right->m_LeftSibling = left->m_Offset;
                right->m_RightSibling = right->m_Offset;

                int mid = (TREE_ORDER - 1) / 2;

                // left half of the keys copied to left
                for (int i = 0; i < mid; i++) {
                    left->m_Keys[i] = m_Keys[i];
                    left->m_UsedCellNumber++;
                }

                for (int i = mid; i < m_UsedCellNumber; i++) {
                    right->m_Keys[i - mid] = m_Keys[i];
                    right->m_UsedCellNumber++;
                }

                m_Keys = std::vector<IndexPayload>(1);

                m_Keys[0] = right->m_Keys[0];

                m_IsLeaf = false;
            } else {
                left->m_IsLeaf = false;
                right->m_IsLeaf = false;

                left->m_Offset = *next_offset;
                right->m_Offset = *next_offset + inner_size;

                *next_offset += 2 * inner_size;

                int mid = (TREE_ORDER - 1) / 2;

                for (int i = 0; i < m_UsedCellNumber / 2; i++) {
                    left->m_Keys[i] = m_Keys[i];
                    left->m_UsedCellNumber++;
                }

                m_Keys = std::vector<IndexPayload>(1);

                m_Keys[0] = right->m_Keys[mid];

                for (int i = mid + 1; i < m_UsedCellNumber; i++) {
                    right->m_Keys[i - mid] = m_Keys[i];
                    right->m_UsedCellNumber++;
                }

                for (int i = 0; i < m_ChildNumber / 2; i++) {

                    left->m_Childs[i] = m_Childs[i];
                }

                for (int i = mid; i < m_ChildNumber; i++) {

                    right->m_Childs[i - mid] = m_Childs[i];
                }
            }

            m_UsedCellNumber = 1;

            // 2 children ptrs only to newly formed left and right

            m_Childs.push_back(left->m_Offset);
            m_Childs.push_back(right->m_Offset);

            m_ChildNumber += 2;

            left->SerializeLeaf(fd);
            right->SerializeLeaf(fd);

            SerializeNode(fd);
            // return midKey;
        }

        bool InsertElement(const IndexPayload& value)
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
                const IndexPayload& e = m_Keys[i];

                // compare values, not index
                if (value.m_Data <= e.m_Data) {
                    m_Keys.resize(m_UsedCellNumber + 1);

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

            m_Keys.push_back(value);

            m_UsedCellNumber++;
            return true;
        }

        void SerializeNode(int fd) const
        {
            assert(!m_IsLeaf);

            assert(m_Offset != 0);

            DbOffset offset = m_Offset;

            lseek(fd, offset, SEEK_SET);

            FileInterface::WriteField(fd, &m_IsLeaf, &offset, DbElemType::DbBool);

            FileInterface::WriteField(fd, &m_UsedCellNumber, &offset, DbElemType::DbUInt8);

            auto childs = std::vector<DbOffset>();

            childs.reserve(m_ChildNumber);

            for (int i = 0; i < m_ChildNumber; i++) {
                childs.emplace_back(m_Childs[i]);
            }

            FileInterface::WriteVec(fd, m_ChildNumber, childs, &offset, DbElemType::DbUInt64);

            offset = lseek(fd, (N - m_ChildNumber) * DB_UINT64_SIZE, SEEK_CUR);

            FileInterface::WriteField(fd, &m_UsedCellNumber, &offset, DbElemType::DbUInt8);

            for (int i = 0; i < m_UsedCellNumber; i++) {
                m_Keys[i].WriteIndexPayload(fd, &offset, m_ElementType);
            }
        }

        void SerializeLeaf(int fd) const
        {
            assert(m_IsLeaf);

            assert(m_Offset != 0);

            DbOffset offset = m_Offset;

            lseek(fd, offset, SEEK_SET);

            DbUInt8 e_size = Convert::TypeToTypeSize(m_ElementType);

            FileInterface::WriteField(fd, &m_IsLeaf, &offset, DbElemType::DbBool);

            FileInterface::WriteField(fd, &m_UsedCellNumber, &offset, DbElemType::DbUInt8);

            for (int i = 0; i < m_UsedCellNumber; i++) {
                m_Keys[i].WriteIndexPayload(fd, &offset, m_ElementType);
            }

            offset = lseek(fd, (N - 1 - m_UsedCellNumber) * (e_size + DB_OFFSET_REPR_SIZE), SEEK_CUR);

            if (DB_OFFSET_REPR_SIZE == 4) {
                FileInterface::WriteField(fd, &m_LeftSibling, &offset, DbElemType::DbUInt);

                FileInterface::WriteField(fd, &m_RightSibling, &offset, DbElemType::DbUInt);
            } else {
                FileInterface::WriteField(fd, &m_LeftSibling, &offset, DbElemType::DbUInt64);

                FileInterface::WriteField(fd, &m_RightSibling, &offset, DbElemType::DbUInt64);
            }
        }

        static Node ReadLeaf(int fd, DbElemType e_type, DbOffset start_offset)
        {
            DbOffset offset = start_offset;

            lseek(fd, offset, SEEK_SET);

            DbUInt8 e_size = Convert::TypeToTypeSize(e_type);

            DbBool is_leaf;

            FileInterface::ReadField(fd, &is_leaf, &offset, DB_BOOL_SIZE);

            assert(is_leaf);

            DbUInt8 used_cell_num;

            FileInterface::ReadField(fd, &used_cell_num, &offset, DB_UINT8_SIZE);

            std::vector<IndexPayload> keys;

            FileInterface::ReadVec(fd, keys, &offset, e_size + DB_OFFSET_REPR_SIZE, used_cell_num);

            for (int i = 0; i < used_cell_num; i++) {
                keys.push_back(IndexPayload(fd, &offset, e_type));
            }

            offset = lseek(fd, (N - 1 - used_cell_num) * e_size, SEEK_CUR);

            DbOffset l_sibling;
            DbOffset r_sibling;

            FileInterface::ReadField(fd, &l_sibling, &offset, DB_UINT64_SIZE);
            FileInterface::ReadField(fd, &r_sibling, &offset, DB_UINT64_SIZE);

            return Node(start_offset, used_cell_num, keys, l_sibling, r_sibling, e_type);
        }

        static Node ReadInternalNode(int fd, DbElemType e_type, DbOffset start_offset)
        {
            DbOffset offset = start_offset;

            lseek(fd, offset, SEEK_SET);

            DbUInt8 e_size = Convert::TypeToTypeSize(e_type);

            DbBool is_leaf;

            FileInterface::ReadField(fd, &is_leaf, &offset, DB_BOOL_SIZE);

            assert(!is_leaf);

            DbUInt8 child_num;

            FileInterface::ReadField(fd, &child_num, &offset, DB_UINT8_SIZE);

            std::vector<DbOffset> childs;

            FileInterface::ReadVec(fd, childs, &offset, DB_UINT64_SIZE, child_num);

            offset = lseek(fd, (N - child_num) * DB_UINT64_SIZE, SEEK_CUR);
            DbUInt8 used_cell_num;

            FileInterface::ReadField(fd, &used_cell_num, &offset, DB_UINT8_SIZE);

            std::vector<IndexPayload> keys;

            for (int i = 0; i < used_cell_num; i++) {
                keys.push_back(IndexPayload(fd, &offset, e_type));
            }

            return Node(start_offset, child_num, childs, used_cell_num, keys, e_type);
        }

        // Constructor for root
        Node(int fd, DbElemType e_type, DbOffset starts_at)
        {
            // Read child number
            DbOffset offset = starts_at;

            DbBool is_leaf;

            DbUInt8 child_num = 0;

            DbUInt8 used_cell_num = 0;

            auto childs = new std::vector<DbOffset>;

            auto keys = new std::vector<IndexPayload>;

            DbOffset l_sibling;

            DbOffset r_sibling;

            DbUInt8 e_size = Convert::TypeToTypeSize(e_type);

            FileInterface::ReadField(fd, &is_leaf, &offset, DB_BOOL_SIZE);

            if (!is_leaf) {
                FileInterface::ReadField(fd, &child_num, &offset, DB_UINT8_SIZE);

                FileInterface::ReadField(fd, &used_cell_num, &offset, DB_UINT8_SIZE);

                FileInterface::ReadVec(fd, *childs, &offset, DB_OFFSET_REPR_SIZE, child_num);

                offset = lseek(fd, (N - child_num) * DB_UINT64_SIZE, SEEK_CUR);

                for (int i = 0; i < used_cell_num; i++) {
                    keys->push_back(IndexPayload(fd, &offset, e_type));
                }

            } else {

                FileInterface::ReadField(fd, &used_cell_num, &offset, DB_UINT8_SIZE);

                for (int i = 0; i < used_cell_num; i++) {
                    keys->push_back(IndexPayload(fd, &offset, e_type));
                }

                offset = lseek(fd, (N - 1 - used_cell_num) * (e_size + DB_NB_ELEMT_INT), SEEK_CUR);

                FileInterface::ReadField(fd, &l_sibling, &offset, DB_OFFSET_REPR_SIZE);

                FileInterface::ReadField(fd, &r_sibling, &offset, DB_OFFSET_REPR_SIZE);
            }

            m_ElementType = e_type;

            m_Offset = starts_at;

            m_IsLeaf = is_leaf;

            m_ChildNumber = child_num;

            m_UsedCellNumber = used_cell_num;

            m_Childs = *childs;

            m_Keys = *keys;

            m_LeftSibling = l_sibling;

            m_RightSibling = r_sibling;
        }

        Node() = default;

        // Constructor for leaves
        Node(DbOffset offset, DbUInt8 used_cell, const std::vector<IndexPayload>& keys, DbOffset l_sibling, DbOffset r_sibling, DbElemType e_type)
            : m_IsLeaf(true)
            , m_ElementType(e_type)
            , m_Offset(offset)
            , m_UsedCellNumber(used_cell)
            , m_Keys(keys)
            , m_LeftSibling(l_sibling)
            , m_RightSibling(r_sibling)
        {
        }

        // Constructor for inner leaves
        Node(DbOffset offset, DbUInt8 child_num, const std::vector<DbOffset>& childs, DbUInt8 used_cell, const std::vector<IndexPayload>& keys, DbElemType e_type)
            : m_IsLeaf(false)
            , m_ElementType(e_type)
            , m_Offset(offset)
            , m_ChildNumber(child_num)
            , m_Childs(childs)
            , m_UsedCellNumber(used_cell)
            , m_Keys(keys)
        {
        }
    };

    void WriteRoot(DbOffset root_offset)
    {
        BPlusTree<N>::Node root = Node(root_offset, 0, std::vector<IndexPayload>(N - 1), root_offset, root_offset, m_ElementType);

        root.SerializeLeaf(m_Fd);

        m_NextOffset += m_InnerSize;
    }

    BPlusTree<N>::Node FindRoot(DbOffset offset)
    {

        lseek(m_Fd, offset, SEEK_SET);

        return BPlusTree<N>::Node(m_Fd, m_ElementType, offset);
    }

    bool IsLeaf(DbOffset offset)
    {
        lseek(m_Fd, offset, SEEK_SET);

        bool is_leaf = false;

        read(m_Fd, &is_leaf, DB_BOOL_SIZE);

        return is_leaf;
    }

    BPlusTree<N>::Node SearchLeaf(BPlusTree<N>::Node& n, const IndexPayload& k)
    {
        Node& curr = n;

        while (!curr.m_IsLeaf) {

            bool found = false;

            for (int i = 0; i < n.m_ChildNumber; i++) {

                if (k.m_Data < n.m_Keys[i].m_Data) {

                    if (IsLeaf(n.m_Childs[i])) {
                        return BPlusTree<N>::Node::ReadLeaf(m_Fd, m_ElementType, n.m_Childs[i]);
                    }

                    found = true;

                    curr = BPlusTree<N>::Node::ReadInternalNode(m_Fd, m_ElementType, n.m_Childs[i]);
                    break;
                }
            }

            if (!found) {
                if (IsLeaf(n.m_Childs[n.m_ChildNumber - 1])) {
                    return BPlusTree<N>::Node::ReadLeaf(m_Fd, m_ElementType, n.m_Childs[n.m_ChildNumber - 1]);
                }

                curr = BPlusTree<N>::Node::ReadInternalNode(m_Fd, m_ElementType, n.m_Childs[n.m_ChildNumber - 1]);
            }
        }

        return n;
    }

    bool Search(BPlusTree<N>::Node& root, const IndexPayload& k)
    {
        const BPlusTree<N>::Node leaf = SearchLeaf(root, k);

        for (DbKey i = 0; i < leaf.m_Keys.size(); i++) {
            if (leaf.m_Keys[i].m_Data == k.m_Data)
                return true;
        }

        return false;
    }

    void Insert(BPlusTree<N>::Node& root, const IndexPayload& value)
    {
        BPlusTree<N>::Node leaf = SearchLeaf(root, value);

        if (leaf.InsertElement(value)) {
            // Replace and reorder the values with the new one and serialize back

            leaf.SerializeLeaf(m_Fd);

            return;
        }

        if (leaf.m_ChildNumber < N) {
            leaf.Split(m_Fd, &m_NextOffset, m_LeafSize, m_InnerSize);

            Insert(leaf, value); // cant fail, we just splitted
        } else {
            throw std::runtime_error("los problemos");
        }
    }
};

}

#endif //! BPL
