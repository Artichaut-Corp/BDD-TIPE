#include "common.h"
#include "types.h"

#include <cstdio>
#include <unistd.h>
#include <vector>

#ifndef B_TREE_H
#define B_TREE_H

namespace Database::Storing {

template <DbInt8 N, DbInt8 S>
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

        DbInt8 m_UsedCellNumber = 0;

        DbInt8 m_ChildNumber = 0;

        DbInt64 m_LeftSibling = 0;
        DbInt64 m_RightSibling = 0;

        std::vector<ColumnData> m_Keys; // Size N - 1 at most
        std::vector<DbInt64> m_Childs; // N at most

        bool IsFull() const;

        void InsertValRange(int fd);

        void SerializeNode(int fd);

        void SerializeLeaf(int fd);

        static Node ReadLeaf(int fd);

        static Node ReadInternalNode(int fd);

        // Constructor for root
        Node(int fd)
        {
            // Read child number
            DbInt64 offset;

            lseek64(fd, DB_BOOL_SIZE, SEEK_SET);

            DbInt8 child_num;

            FileInterface::ReadField(fd, &child_num, &offset, DB_INT8_SIZE);

            DbInt8 used_cell_num;

            FileInterface::ReadField(fd, &used_cell_num, &offset, DB_INT8_SIZE);

            std::vector<DbInt64> childs;

            FileInterface::ReadVec(fd, childs, &offset, DB_INT64_SIZE, child_num);

            offset = lseek64(fd, (N - child_num) * DB_INT64_SIZE, SEEK_SET);

            std::vector<DbInt64> keys;

            FileInterface::ReadVec(fd, childs, &offset, S, used_cell_num);

            m_Childs = childs;

            m_ChildNumber = child_num;

            m_UsedCellNumber = used_cell_num;

            m_Keys =


        }

        // Constructor for leafs
    };

    static Node FindRoot(int fd, DbInt64 offset);

    static Node SearchLeaf(int fd, const ColumnData k, Node n);

    static bool Search(int fd, const Node root, const ColumnData k);

    static void Insert(int fd, const Node root, const ColumnData value);
};

}

#endif
