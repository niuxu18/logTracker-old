{
    sprintf(firstLine, "Page %d of %d", cw->GetPage(), m_NumberOfPages);
    curses_move(0,65-strlen(firstLine)-1);
    printw(firstLine);
    }