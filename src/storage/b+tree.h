#include "types.h"

#include <memory>
#include <vector>

#ifndef B_TREE_H
#define B_TREE_H

namespace Database::Utils {

// template <typename T, DbInt8 N>
template <DbInt8 N>
class BPlusTree {

    struct IndexPayload {
        // Used only for
        const DbKey m_Key;
    };

    /*
      struct DataPayload {
          const T m_Data;
          const DbInt8 m_DataSize;
      };
  */

    struct Node {
        DbBool m_IsRoot;

        DbBool m_IsLeaf;

        DbInt8 m_UsedCellNumber;

        std::unique_ptr<Node> m_LeftSibling;
        std::unique_ptr<Node> m_RightSibling;

        std::vector<IndexPayload> m_Keys; // Size N - 1 at most
        std::vector<Node*> m_Childs; // N at most

        bool IsFull() const;

        void InsertValRange();
    };

    Node m_Root;

    DbInt64 m_Offset;

public:
    void Serialize();

    bool Search(const DbKey k) const;

    Node SearchLeaf(const DbKey k, Node n) const;

    void Insert(const DbKey value);
};

}

#endif
