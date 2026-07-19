;************************************
;*       FACTS (INITIAL STATE)      *
;************************************

(deffacts status
                (t-status (critical-counter 0) (error-counter 0) (critical-counter-five 0) (error-counter-five 0) (critical-counter-ten 0) (error-counter-ten 0))
)

;********************************
;             CURRENT           *
;********************************

(deffacts node
                (t-node (nid n0) (loadavg 100) (memory-used 75) (swap 20))
                (t-node (nid n1) (loadavg 300) (memory-used 92) (swap 89))
)

(deffacts cpu-core
                (t-cpu-core (nid n0) (cid c0) (user-load 50) (sys-load 50))
                (t-cpu-core (nid n0) (cid c1) (user-load 85) (sys-load 85))
                (t-cpu-core (nid n1) (cid c0) (user-load 95) (sys-load 95))
                (t-cpu-core (nid n1) (cid c1) (user-load 100) (sys-load 100))
)

(deffacts filesystems
                (t-filesystems (nid n0) (fid froot) (du-free 100) (rtime 100) (wtime 100))
                (t-filesystems (nid n0) (fid fboot) (du-free 100) (rtime 100) (wtime 100))
                (t-filesystems (nid n0) (fid fhome) (du-free 100) (rtime 100) (wtime 100))
                (t-filesystems (nid n1) (fid froot) (du-free 200) (rtime 200) (wtime 200))
                (t-filesystems (nid n1) (fid fboot) (du-free 200) (rtime 200) (wtime 200))
                (t-filesystems (nid n1) (fid fhome) (du-free 200) (rtime 200) (wtime 200))
)

(deffacts network
                (t-network (nid n0) (iid ilo) (rx-bytes 100) (tx-bytes 100) (rx-errors 100) (tx-dropped 100))
                (t-network (nid n0) (iid ieth0) (rx-bytes 100) (tx-bytes 100) (rx-errors 100) (tx-dropped 100))
                (t-network (nid n1) (iid ilo) (rx-bytes 200) (tx-bytes 200) (rx-errors 200) (tx-dropped 200))
                (t-network (nid n1) (iid ieth0) (rx-bytes 200) (tx-bytes 200) (rx-errors 200) (tx-dropped 200))
)

;********************************
;             5 MIN             *
;********************************

(deffacts node-five
                (t-node-five (nid n0) (loadavg-five 100) (memory-used-five 75) (swap-five 20))
                (t-node-five (nid n1) (loadavg-five 300) (memory-used-five 92) (swap-five 89))
)

;********************************
;            10 MIN             *
;********************************

(deffacts node-ten
                (t-node-ten (nid n0) (loadavg-ten 100) (memory-used-ten 75) (swap-ten 20))
                (t-node-ten (nid n1) (loadavg-ten 300) (memory-used-ten 92) (swap-ten 89))
)
