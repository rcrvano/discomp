;********************************
;*       DEFINES TEMPLATES      *
;********************************

(deftemplate t-status
                (slot critical-counter)
                (slot error-counter)
                (slot critical-counter-five)
                (slot error-counter-five)
                (slot critical-counter-ten)
                (slot error-counter-ten)
)

;********************************
;             CURRENT           *
;********************************

(deftemplate t-node
                (slot nid)
                (slot loadavg)
                (slot memory-used)
                (slot swap)
)

(deftemplate t-cpu-core
                (slot nid)
                (slot cid)
                (slot user-load)
                (slot sys-load)
)

(deftemplate t-filesystems
                (slot nid)
                (slot fid)
                (slot du-free)
                (slot rtime)
                (slot wtime)
)

(deftemplate t-network
                (slot nid)
                (slot iid)
                (slot rx-bytes)
                (slot tx-bytes)
                (slot rx-errors)
                (slot tx-dropped)
)

;***************************
;         5 MIN            *
;***************************

(deftemplate t-node-five
                (slot nid)
                (slot loadavg-five)
                (slot memory-used-five)
                (slot swap-five)
)

;***************************
;         10 MIN            *
;***************************

(deftemplate t-node-ten
                (slot nid)
                (slot loadavg-ten)
                (slot memory-used-ten)
                (slot swap-ten)
)



