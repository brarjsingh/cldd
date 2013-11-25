/**
 * Copyright (C) 2010 Control, Logging, and Data Acquisition Daemon
 * http://www.github.com/geoffjay/cldd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

using Config;
using ZMQ;

class Cldd.Application : GLib.Object {

    /* Application data */
    private static GLib.MainLoop loop;
    private static Posix.pid_t pid;
    private static bool done = false;
    private static Context context = new Context ();

    /* Command line arguments */
    private static bool kill = false;
    private static bool daemonize = false;
    private static string cfgfile = null;
    private static string pidfile = null;
    private static bool version = false;

    private const GLib.OptionEntry[] options = {{
        "kill", 'k', 0, OptionArg.NONE, ref kill,
        "Terminate a currently running CLDD instance.", null
    },{
        "daemonize", 'd', 0, OptionArg.NONE, ref daemonize,
        "Send the daemon to the background.", null
    },{
        "config", 'c', 0, OptionArg.STRING, ref cfgfile,
        "Use the configuration file provided.", null
    },{
        "pidfile", 'p', 0, OptionArg.STRING, ref pidfile,
        "Use the PID file provided.", null
    },{
        "version", 'V', 0, OptionArg.NONE, ref version,
        "Display CLDD version number.", null
    },{
        null
    }};

    /* Application settings read from command line or configuration  */
    private Settings settings;

    /* Application configuration data */
    private Config config;

    /* Daemon component of application */
    private Daemon daemon;

    /**
     * Perform initialization.
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

            /* Check if we are called with -k parameter */
            if (kill) {
                /* Kill using SIGINT */
                return daemon.interupt ();
            } else {
                int ret;

                /* Setup CLD */
                Cld.init (args);

                /* If no configuration file was given use the system one */
                if (cfgfile == null) {
                    cfgfile = Path.build_filename (DATADIR, "cldd.xml");
                }

                config = new Cldd.Config (cfgfile);

                /* Check that the daemon is not rung twice a the same time */

                /* Prepare for return value passing from the initialization procedure of the daemon process */
                ret = daemon.init ();

                return ret;
            }
        }

        return 0;
    }

    /**
     * XXX fix this to spawn a thread for the main daemon body
     */
    public static void run () {
/*
 *        var responder = Socket.create (context, SocketType.REP);
 *        //var responder_service = "tcp://*:%d".printf (port);
 *        var responder_service = "tcp://*:%d".printf (5555);
 *        responder.bind (responder_service);
 *
 *        while (!done) {
 *            var request = Msg ();
 *            request.recv (responder);
 *            Daemon.log (Daemon.LogPriority.INFO, "Received message: %s", request.data);
 *
 *            [> do something <]
 *            Posix.sleep (1);
 *
 *            [> reply if necessary <]
 *            var reply = Msg.with_data (request.data);
 *            reply.send (responder);
 *        }
 */

        loop = new MainLoop ();

        /* Launch daemon */
        deamon.launch ();
        daemon.closed.connect (() => { loop.quit (); });

        loop.run ();
    }

    public static void quit () {
        /* Cleanup */
        daemon.close ();
        daemon.pid_file_remove ();
        done = true;
    }

    /**
     * Daemon entry point.
     */
    public static int main (string[] args) {

        int _ret = 0;

        if ((_ret = init (args)) != 0)
            return _ret;

        /* XXX daemon creation goes here */

        Posix.signal (Posix.SIGINT, quit);

        run ();

        return 0;
    }
}
