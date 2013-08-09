class Cldd.Daemon : GLib.Object {

    public void run () {
        stdout.printf ("Hello World!\n");
    }
}

public static int main (string[] args) {

    Cldd.Daemon daemon = new Cldd.Daemon ();
    daemon.run ();
    return 0;
}
