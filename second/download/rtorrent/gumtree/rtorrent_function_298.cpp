virtual char* print(char* first, char* last, Canvas::attributes_list* attributes, void* object) {
    if (object == NULL)
      return first;

    if (m_slot(reinterpret_cast<arg1_type>(object)))
      return m_branch1 != NULL ? m_branch1->print(first, last, attributes, object) : first;
    else
      return m_branch2 != NULL ? m_branch2->print(first, last, attributes, object) : first;
  }