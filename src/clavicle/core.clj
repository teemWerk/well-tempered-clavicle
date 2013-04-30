(require '[overtone.live :refer :all])
(require '[overtone.synth.sampled-piano :refer :all])
(require '[overtone.inst.drum :refer :all])
(require '[overtone.inst.synth :refer :all])
(require '[overtone.synth.stringed :refer :all])
(require '[overtone.samples.piano :refer [index-buffer]])

(def server (osc-server 50501))
(def client (osc-client "localhost" 50501))
(def of-client (osc-client "localhost" 12346))

(defsynth p-samp-piano
  [note 60 level 1 pos 0 rate 1 loop? 0
   attack 0 decay 1 sustain 1 release 0.1 curve -4 gate 1 amp 1]
  (let [buf (index:kr (:id index-buffer) note)
        env (env-gen (adsr attack decay sustain release level curve)
                     :gate gate
                     :action FREE)]
    (out 0 (pan2 (* env amp (scaled-play-buf 2 buf :level level :loop loop? :action FREE)) pos))))

(definst p-ks1
  [note  {:default 60  :min 10   :max 120  :step 1}
   amp   {:default 0.8 :min 0.01 :max 0.99 :step 0.01}
   dur   {:default 2   :min 0.1  :max 4    :step 0.1}
   decay {:default 30  :min 1    :max 50   :step 1}
   coef  {:default 0.3 :min 0.01 :max 2    :step 0.01}
   pos 0]
  (let [freq (midicps note)
        noize (* 0.8 (white-noise))
        dly (/ 1.0 freq)
        plk   (pluck noize 1 (/ 1.0 freq) dly
                     decay
                     coef)
        dist (distort plk)
        filt (rlpf dist (* 12 freq) 0.6)
        clp (clip2 filt 0.8)
        reverb (free-verb clp 0.4 0.8 0.2)]
     (pan2 (* (env-gen (perc 0.0001 dur) :action FREE) reverb) pos amp)))


(osc-handle server "/test" (fn [msg] (println "MSG: " (get msg :args))))

(def input-pitches (concat
                    (degrees->pitches [:i :ii :iii :iv :v :vi :vii] :diatonic :F4)
                    (degrees->pitches [:i :iii :iv :vii] :mixolydian :C4)
                    (degrees->pitches [:ii :iv :v :vi :vii] :diatonic :F5)))

(def in-prob-map {0 0.4, 1 0.25, -1 0.15, 2 0.1, -2 0.1})

(osc-handle server "/note" (fn [msg]
                             (let [arg (get msg :args)
                                   degree (nth input-pitches (+ (weighted-choose in-prob-map) (first arg)) )
                                   vol (second arg)
                                   pos (nth arg 2)]
                               (p-samp-piano degree (* vol 0.2) pos))))


(def phrase [:i :iii :iv])
(def pitches (concat
              (degrees->pitches phrase :mixolydian :C4)
              (degrees->pitches phrase :mixolydian :C2)
              (degrees->pitches phrase :diatonic :F3)
              (degrees->pitches phrase :diatonic :F2)))

(def my-drum ping)
(inst-volume! my-drum 0.6)
(osc-handle server "/percussion" (fn [msg]
                                   (let [arg (get msg :args)
                                         tone (choose pitches)
                                         pos (second arg)]
                                     (inst-pan! my-drum pos)
                                     (ping tone 0.12 2))))


(def m (metronome 38))

(defn player [beat]
  (let [next-beat (inc beat)]
    (let [vol (ranged-rand 0 0.4)
          num (rand-int 12)
          pitch (nth pitches num)
          pos (scale-range (+ (- 1 (rand))  num) 0 12 -1 1)]
      (apply-at (m beat) #'osc-send [of-client "/drop" (int (scale-range vol 0 0.3 10 20)) (float pos)])
      (at (m beat)
          (p-ks1 pitch vol 4 100 0.01 pos)))
    (let [vol (ranged-rand 0 0.8)
          num (rand-int 12)
          pitch (nth pitches num)
          pos (scale-range (+ (- 1 (rand)) num) 0 12 -1 1)
          swing (ranged-rand 0 0.5)]
      (apply-at (m (+ 0.5 beat swing)) #'osc-send [of-client "/drop" (int (scale-range vol 0 0.8 10 20)) (float pos)])
      (at (m (+ 0.5 beat swing))
          (p-ks1 pitch vol 4 100 0.01 pos)))
    (apply-at (m next-beat) #'player [next-beat])))

(player (m))

(comment
  (dropper [beat]
           (apply-at (m beat) #'osc-send [of-client "/drop" ])))
