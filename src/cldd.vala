/**
 * libdaemon example: http://stuff.mit.edu/afs/athena.mit.edu/astaff/source/src-9.3/third/libdaemon/doc/reference/html/testd_8c-example.html
 */

using Posix;
using Config;

class Cldd.Application : GLib.Object {

    private static GLib.MainLoop loop;
    private static pid_t pid;
    private static string[] args;
    private static int fd;
    private static bool done = false;
    private static fd_set fds;

    public Application (string[] args) {
        this.args = args;
    }

    public int init () {
        /* XXX fix this to use OptionContext for command line processing */

        /* Set indetification string for the daemon for both syslog and PID file */
        Daemon.pid_file_ident = Daemon.log_ident = Daemon.ident_from_argv0 (args[0]);

        /* Check if we are called with -k parameter */
        if (args.length >= 2 && (Posix.strcmp (args[1], "-k") != 0)) {
            int ret;

            /* Kill using SIGINT */
            if ((ret = Daemon.pid_file_kill_wait (Daemon.Sig.INT, 5)) < 0)
                Daemon.log (Daemon.LogPriority.WARNING, "Failed to kill daemon.");

            return ret < 0 ? 1 : 0;
        }

        /* Check that the daemon is not rung twice a the same time */
        if ((pid = Daemon.pid_file_is_running ()) >= 0) {
            Daemon.log (Daemon.LogPriority.ERR, "Daemon already running on PID file %u", pid);
            return 1;

        }

        /* Prepare for return value passing from the initialization procedure of the daemon process */
        Daemon.retval_init ();

        return 0;
    }

    /**
     * XXX fix this to spawn a thread for the main daemon body
     */
    public void run () {
        while (!done) {
            Posix.sleep (10);
            Daemon.log (Daemon.LogPriority.INFO, "Went through again...");
        }
    }

    public void quit () {
        /* Do a cleanup */
        Daemon.log (Daemon.LogPriority.INFO, "Exiting...");

        Daemon.signal_done ();
        Daemon.pid_file_remove ();

        done = true;
    }


    public static int main (string[] args) {

        int _ret = 0;

        /* XXX might not be correct using an object */
        var daemon = new Cldd.Application (args);

        if ((_ret = daemon.init ()) != 0)
            return _ret;

        /* Do the fork */
        if ((pid = Daemon.fork ()) < 0) {

            /* Exit on error */
            Daemon.retval_done ();
            return 1;

        } else if (pid != 0) {
            /* The parent */
            int ret;

            /* Wait for 20 seconds for the return value passed from the daemon process */
            if ((ret = Daemon.retval_wait (20)) < 0) {
                Daemon.log (Daemon.LogPriority.ERR, "Could not recieve return value from daemon process.");
                return 255;
            }

            Daemon.log (ret != 0 ? Daemon.LogPriority.ERR : Daemon.LogPriority.INFO, "Daemon returned %i as return value.", ret);
            return ret;

        } else {
            /* The daemon */

            /* Create the PID file */
            if (Daemon.pid_file_create () < 0) {
                Daemon.log (Daemon.LogPriority.ERR, "Could not create PID file (%s).", Posix.strerror (Posix.errno));

                /* Send the error condition to the parent process */
                Daemon.retval_send (1);
                daemon.quit ();
            }

            /* Initialize signal handling */
            if (Daemon.signal_init (Daemon.Sig.INT, Daemon.Sig.QUIT, Daemon.Sig.HUP, 0) < 0) {
                Daemon.log (Daemon.LogPriority.ERR, "Could not register signal handlers (%s).", Posix.strerror (Posix.errno));
                Daemon.retval_send (2);
                daemon.quit ();
            }

            /* ... do some further init work here */

            /* Send OK to parent process */
            Daemon.retval_send (0);

            Daemon.log (Daemon.LogPriority.INFO, "Sucessfully started");

            /* This doesn't work with vala-0.14
             *GLib.Unix.signal_add (Posix.SIGINT, () => {
             *    daemon.quit ();
             *    return 0;
             *});
             */

            Posix.signal (Posix.SIGINT, daemon.quit);

            daemon.run ();

            return 0;
        }

        return 0;
    }
}
