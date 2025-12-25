#include "common.h"
#include "types.h"

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
    struct Node {
        DbBool m_IsLeaf = false;

        DbUInt8 m_UsedCellNumber = 0;

        DbUInt8 m_ChildNumber = 0;

        DbUInt64 m_LeftSibling = 0;
        DbUInt64 m_RightSibling = 0;

        std::vector<ColumnData> m_Keys; // Size N - 1 at most
        std::vector<DbUInt64> m_Childs; // N at most

        bool IsFull() const;

        void InsertElement(const ColumnData value);

        void SerializeNode(int fd);

        void SerializeLeaf(int fd);

        static Node ReadLeaf(int fd, DbInt64 offset);

        static Node ReadInternalNode(int fd, DbInt64 offset);

        // Constructor for root
        Node(int fd, DbUInt8 e_size)
        {
            // Read child number
            DbUInt64 offset;

            lseek(fd, DB_BOOL_SIZE, SEEK_SET);

            DbUInt8 child_num;

            FileInterface::ReadField(fd, &child_num, &offset, DB_UINT8_SIZE);

            DbUInt8 used_cell_num;

            FileInterface::ReadField(fd, &used_cell_num, &offset, DB_UINT8_SIZE);

            std::vector<DbUInt64> childs;

            FileInterface::ReadVec(fd, childs, &offset, DB_UINT64_SIZE, child_num);

            offset = lseek(fd, (N - child_num) * DB_UINT64_SIZE, SEEK_SET);

            std::vector<ColumnData> keys;

            FileInterface::ReadVec(fd, childs, &offset, e_size, used_cell_num);

            m_Childs = childs;

            m_ChildNumber = child_num;

            m_UsedCellNumber = used_cell_num;

            m_Keys = keys;
        }

        // Constructor for leafs
    };

    static Node FindRoot(int fd, DbUInt64 offset);

    static Node SearchLeaf(int fd, const ColumnData k, Node n);

    static bool Search(int fd, const Node root, const ColumnData k);

    static void Insert(int fd, const Node root, const ColumnData value);
};

}

#endif
