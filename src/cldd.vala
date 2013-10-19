/**
 */

using Config;
using Posix;
using ZMQ;

/**
 * NOTE:
 * - tried to launch a thread, daemon dies immediately likely could be solved by
 *   starting the GLib loop
 */

class Cldd.Application : GLib.Object {

    private static pid_t pid;

    private static GLib.MainLoop loop;
    private static bool done = false;
    private static Context context = new Context ();

    /* Command line arguments. */
    private static bool kill = false;
    private static string cfgfile = null;
    private static bool version = false;

    private const GLib.OptionEntry[] options = {{
        "kill", 'k', 0, OptionArg.NONE, ref kill,
        "Terminate a currently running CLDD instance.", null
    },{
        "config", 'c', 0, OptionArg.STRING, ref cfgfile,
        "Use the configuration file provided.", null
    },{
        "version", 'V', 0, OptionArg.NONE, ref version,
        "Display CLDD version number.", null
    },{
        null
    }};

    /* XXX these will be loaded using configuration */
    private int _port = 5555;
    public int port {
        get { return _port; }
        set { _port = value; }
    }

    /**
     * Perform daemon initialization.
     */
    public static int init (string[] args) {

        try {
            var opt_context = new OptionContext (PACKAGE_NAME);
            opt_context.set_help_enabled (true);
            opt_context.add_main_entries (options, null);
            opt_context.parse (ref args);
        } catch (OptionError e) {
            GLib.stdout.printf ("error: %s\n", e.message);
            GLib.stdout.printf ("Run `%s --help' to see a full list of available command line options\n", args[0]);
            return 1;
        }

        if (version) {
            GLib.stdout.printf ("%s\n", PACKAGE_VERSION);
        } else {
            /* Set indetification string for the daemon for both syslog and PID file */
            Daemon.pid_file_ident = Daemon.log_ident = Daemon.ident_from_argv0 (args[0]);

            /* Check if we are called with -k parameter */
            if (kill) {
                int ret;

                /* Kill using SIGINT */
                if ((ret = Daemon.pid_file_kill_wait (Daemon.Sig.INT, 5)) < 0)
                    Daemon.log (Daemon.LogPriority.WARNING, "Failed to kill daemon.");

                return ret < 0 ? 1 : 0;
            } else {
                if (cfgfile == null) {
                    cfgfile = Path.build_filename (DATADIR, "cldd.xml");
                }

                /* Check that the daemon is not rung twice a the same time */
                if ((pid = Daemon.pid_file_is_running ()) >= 0) {
                    Daemon.log (Daemon.LogPriority.ERR, "Daemon already running on PID file %u", pid);
                    return 1;
                }

                /* Prepare for return value passing from the initialization procedure of the daemon process */
                Daemon.retval_init ();
            }
        }

        return 0;
    }

    /**
     * XXX fix this to spawn a thread for the main daemon body
     */
    public static void run () {
        var responder = Socket.create (context, SocketType.REP);
        //var responder_service = "tcp://*:%d".printf (port);
        var responder_service = "tcp://*:%d".printf (5555);
        responder.bind (responder_service);

        while (!done) {
            var request = Msg ();
            request.recv (responder);
            Daemon.log (Daemon.LogPriority.INFO, "Received message: %s", request.data);

            /* do something */
            Posix.sleep (1);

            /* reply if necessary */
            var reply = Msg.with_data (request.data);
            reply.send (responder);
        }
    }

    public static void quit () {
        /* Do a cleanup */
        Daemon.log (Daemon.LogPriority.INFO, "Exiting...");

        Daemon.signal_done ();
        Daemon.pid_file_remove ();

        done = true;
    }

    /**
     * Daemon entry point.
     */
    public static int main (string[] args) {

        int _ret = 0;

        if ((_ret = init (args)) != 0)
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
                quit ();
            }

            /* Initialize signal handling */
            if (Daemon.signal_init (Daemon.Sig.INT, Daemon.Sig.QUIT, Daemon.Sig.HUP, 0) < 0) {
                Daemon.log (Daemon.LogPriority.ERR, "Could not register signal handlers (%s).", Posix.strerror (Posix.errno));
                Daemon.retval_send (2);
                quit ();
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

            Posix.signal (Posix.SIGINT, quit);

            run ();

            return 0;
        }
    }
}
