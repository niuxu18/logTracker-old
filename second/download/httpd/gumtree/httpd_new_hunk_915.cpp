                ap_signal_parent(SIGNAL_PARENT_RESTART);
            }
        }
    }

    if (success)
        fprintf(stderr,"The %s service has %s.\n", mpm_display_name,
               signal ? "restarted" : "stopped");
    else
        fprintf(stderr,"Failed to %s the %s service.\n",
               signal ? "restart" : "stop", mpm_display_name);
}
