#ifndef STACK_H

#define STACK_H

#include "linked_list.h"
namespace Database::Utils {

template<typename T>
class Stack {
    LinkedList<T> m_Data;

public:
Stack() : m_Data(LinkedList<T>())
{
}

bool empty() { return m_Data.is_empty(); }

void push(T element) {
    m_Data.append(element);
}

T pop() {
     T value = m_Data.get_first().value();

     m_Data.advance();

     return value;
}

};

} // namespace Database::Utils


#endif
