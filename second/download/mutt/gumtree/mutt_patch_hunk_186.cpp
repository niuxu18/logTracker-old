       addstr (_(" (S/MIME)"));
   }
 
   if (option (OPTCRYPTOPPORTUNISTICENCRYPT) && (msg->security & OPPENCRYPT))
       addstr (_(" (OppEnc mode)"));
 
-  clrtoeol ();
-  move (HDR_CRYPTINFO, 0);
-  clrtoeol ();
+  mutt_window_clrtoeol (MuttIndexWindow);
+  mutt_window_move (MuttIndexWindow, HDR_CRYPTINFO, 0);
+  mutt_window_clrtoeol (MuttIndexWindow);
 
   if ((WithCrypto & APPLICATION_PGP)
       && (msg->security & APPLICATION_PGP) && (msg->security & SIGN))
-    printw ("%s%s", _(" sign as: "), PgpSignAs ? PgpSignAs : _("<default>"));
+    printw (TITLE_FMT "%s", _("sign as: "), PgpSignAs ? PgpSignAs : _("<default>"));
 
   if ((WithCrypto & APPLICATION_SMIME)
       && (msg->security & APPLICATION_SMIME) && (msg->security & SIGN)) {
-      printw ("%s%s", _(" sign as: "), SmimeDefaultKey ? SmimeDefaultKey : _("<default>"));
+      printw (TITLE_FMT "%s", _("sign as: "), SmimeDefaultKey ? SmimeDefaultKey : _("<default>"));
   }
 
   if ((WithCrypto & APPLICATION_SMIME)
       && (msg->security & APPLICATION_SMIME)
       && (msg->security & ENCRYPT)
       && SmimeCryptAlg
       && *SmimeCryptAlg) {
-      mvprintw (HDR_CRYPTINFO, 40, "%s%s", _("Encrypt with: "),
+    mutt_window_mvprintw (MuttIndexWindow, HDR_CRYPTINFO, 40, "%s%s", _("Encrypt with: "),
 		NONULL(SmimeCryptAlg));
   }
 }
 
 
 #ifdef MIXMASTER
 
 static void redraw_mix_line (LIST *chain)
 {
   int c;
   char *t;
 
-  mvaddstr (HDR_MIX, 0,     "     Mix: ");
+  /* L10N: "Mix" refers to the MixMaster chain for anonymous email */
+  mutt_window_mvprintw (MuttIndexWindow, HDR_MIX, 0, TITLE_FMT, _("Mix: "));
 
   if (!chain)
   {
     addstr ("<no chain defined>");
-    clrtoeol ();
+    mutt_window_clrtoeol (MuttIndexWindow);
     return;
   }
   
   for (c = 12; chain; chain = chain->next)
   {
     t = chain->data;
     if (t && t[0] == '0' && t[1] == '\0')
       t = "<random>";
     
-    if (c + mutt_strlen (t) + 2 >= COLS)
+    if (c + mutt_strlen (t) + 2 >= MuttIndexWindow->cols)
       break;
 
     addstr (NONULL(t));
     if (chain->next)
       addstr (", ");
 
