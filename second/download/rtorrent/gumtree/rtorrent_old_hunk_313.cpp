    return;

  worker_thread->poll()->remove_read(this);
  worker_thread->poll()->insert_write(this);

  if (m_parent->log_fd() >= 0) {
    // Clean up logging, this is just plain ugly...
    //    write(m_logFd, "\n---\n", sizeof("\n---\n"));
    write(m_parent->log_fd(), m_buffer, m_bufferSize);
    write(m_parent->log_fd(), "\n---\n", sizeof("\n---\n"));
  }

  // Close if the call failed, else stay open to write back data.
  if (!m_parent->receive_call(this, m_body, m_bufferSize - std::distance(m_buffer, m_body)))
    close();

  return;

