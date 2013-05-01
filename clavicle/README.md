# clavicle

A clojure script for Overtone to play music with an accompanying application made in openFrameworks. The two communicate over OSC.

## Usage

Standard lein deps, the run a REPL (e.g. lein repl) and load core.clj (load-file "src/clavicle/core.clj")
This takes care of booting up overtone and the like, so no need to do anything. You can then manipulate
the script however you please, the author is partial of [Sam Aaron's excellent EMACS Live](http://overtone.github.io/emacs-live/).
For it to be useful, boot up the openFrameworks app included in the repo.

## License

Copyright © 2013 Timothy Cantwell

Distributed under the Eclipse Public License, the same as Clojure.
