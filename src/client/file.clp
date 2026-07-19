;*******************
;*      RULES      *
;*******************

;******************
;*     STATUS     *
;******************

;increases critical-counter or error-counter in status for current, 5 min or 10 min state

(defrule update-status-error
                (update status error)
                ?counter <- (t-status (error-counter ?error-counter))
                ?upds <- (update status error)
=>
                (modify ?counter (error-counter (+ ?error-counter 1)))            
                (retract ?upds)
)

(defrule update-status-critical
                (update status critical)
                ?counter <- (t-status (critical-counter ?critical-counter))
                ?upds <- (update status critical)
=>
                (modify ?counter (critical-counter (+ ?critical-counter 1)))            
                (retract ?upds)
)

(defrule update-status-error-five
                (update status error five)
                ?counter <- (t-status (error-counter-five ?error-counter-five))
                ?upds <- (update status error five)
=>
                (modify ?counter (error-counter-five (+ ?error-counter-five 1)))
                (retract ?upds)
)

(defrule update-status-critical-five
                (update status critical five)
                ?counter <- (t-status (critical-counter-five ?critical-counter-five))
                ?upds <- (update status critical five)
=>
                (modify ?counter (critical-counter-five (+ ?critical-counter-five 1)))
                (retract ?upds)
)

(defrule update-status-critical-ten
                (update status critical ten)
                ?counter <- (t-status (critical-counter-ten ?critical-counter-ten))
                ?upds <- (update status critical ten)
=>
                (modify ?counter (critical-counter-ten (+ ?critical-counter-ten 1)))
                (retract ?upds)
)

(defrule update-status-error-ten
                (update status error ten)
                ?counter <- (t-status (error-counter-ten ?error-counter-ten))
                ?upds <- (update status error ten)
=>
                (modify ?counter (error-counter-ten (+ ?error-counter-ten 1)))
                (retract ?upds)
)

;**********************
;*    MESSAGES        *
;**********************

(defrule error-message
                (t-status (error-counter ?error-counter) (error-counter-five ?error-counter-five) (error-counter-ten ?error-counter-ten))
                (and
                    (test (> ?error-counter 0))
                    (test (> ?error-counter-five 0))
                    (test (> ?error-counter-ten 0))
                )
                ?err <- (error $?)
=>
                (printout t ?err crlf)
)

(defrule critical-message
                (t-status (critical-counter ?critical-counter) (critical-counter-five ?critical-counter-five) (critical-counter-ten ?critical-counter-ten))
                (and
                    (test (> ?critical-counter 1))
                    (test (> ?critical-counter-five 0))
                    (test (> ?critical-counter-ten 0))
                )
                ?cr <- (critical $?)
=>
                (printout t ?cr crlf)
)

;******************************************
;*       CPU CORES (USER LOAD)            *
;******************************************

(defrule cpu-core-user-load-error
                (t-cpu-core (nid ?nid) (cid ?cid) (user-load ?user-load)) 
                (test (= ?user-load 100)) 
=>
                (assert (error node ?nid core ?cid user-load ?user-load))
                (assert (update status error))
)


(defrule cpu-core-user-load-critical     
                (t-cpu-core (nid ?nid) (cid ?cid) (user-load ?user-load))
                (and
			(test (>= ?user-load 90))
			(test (<= ?user-load 99))
		)
=>
                (assert (critical node ?nid core ?cid user-load ?user-load))
		(assert (update status error))
)

(defrule cpu-core-user-load-warning
                (t-cpu-core (nid ?nid) (cid ?cid) (user-load ?user-load))
                (and
                        (test (>= ?user-load 80))
                        (test (<= ?user-load 89))
                )
=>
                (assert (warning node ?nid core ?cid user-load ?user-load))
)

(defrule cpu-core-user-load-good
                (t-cpu-core (nid ?nid) (cid ?cid) (user-load ?user-load))
                (test (<= ?user-load 79))
=>
                (assert (good node ?nid core ?cid user-load ?user-load))
)

;*******************************************
;*       CPU CORES (SYSTEM LOAD)           *
;*******************************************

(defrule cpu-core-sys-load-error
                (t-cpu-core (nid ?nid) (cid ?cid) (sys-load ?sys-load))
                (test (= ?sys-load 100))
=>
                (assert (error node ?nid core ?cid sys-load ?sys-load))
		(assert (update status error))
)

(defrule cpu-core-sys-load-critical
                (t-cpu-core (nid ?nid) (cid ?cid) (sys-load ?sys-load))
                (and
                        (test (>= ?sys-load 90))
                        (test (<= ?sys-load 99))
                )
=>
                (assert (critical node ?nid core ?cid sys-load ?sys-load))
		(assert (update status critical))
)

(defrule cpu-core-sys-load-warning
                (t-cpu-core (nid ?nid) (cid ?cid) (sys-load ?sys-load))
                (and
                        (test (>= ?sys-load 80))
                        (test (<= ?sys-load 89))
                )
=>
                (assert (warning node ?nid core ?cid sys-load ?sys-load))
)

(defrule cpu-core-sys-load-good
                (t-cpu-core (nid ?nid) (cid ?cid) (sys-load ?sys-load))
                (test (<= ?sys-load 79))
=>
                (assert (good node ?nid core ?cid sys-load ?sys-load))
)

;****************************
;*         LOADAVG          *
;****************************

(defrule node-loadavg-error
                (t-node (nid ?nid) (loadavg ?loadavg))
                (test (= ?loadavg 300))
=>
                (assert (error node ?nid loadavg ?loadavg))
		(assert (update status error))
                (assert (loadavg 5 minute check))
)

(defrule node-loadavg-critical
                (t-node (nid ?nid) (loadavg ?loadavg))
                (and
			(test (>= ?loadavg 200))
			(test (<= ?loadavg 299))
		)
=>
                (assert (critical node ?nid loadavg ?loadavg))
		(assert (update status critical))
                (assert (loadavg 5 minute check))
)

(defrule node-loadavg-warning
                (t-node (nid ?nid) (loadavg ?loadavg))
                (and
			(test (>= ?loadavg 100))
                	(test (<= ?loadavg 199))
		)
=>
                (assert (warning node ?nid loadavg ?loadavg))
) 

(defrule node-loadavg-good
                (t-node (nid ?nid) (loadavg ?loadavg))
                (test (<= ?loadavg 99))
=>
                (assert (good node ?nid loadavg ?loadavg))
)

;**************************
;*        MEMORY          *
;**************************

(defrule node-memory-error
                (t-node (nid ?nid) (memory-used ?memory-used))
                (test (= ?memory-used 100))
=>
                (assert (error node ?nid memory-used ?memory-used))
		(assert (update status error))
                (assert (memory 5 minute check))
)

(defrule node-memory-critical
                (t-node (nid ?nid) (memory-used ?memory-used))
                (and
			(test (>= ?memory-used 90))
			(test (<= ?memory-used 99))
		)
=>                                                
                (assert (critical node ?nid memory-used ?memory-used))
		(assert (update status critical))
                (assert (memory 5 minute check))
)

(defrule node-memory-warning
                (t-node (nid ?nid) (memory-used ?memory-used))
                (and
                        (test (>= ?memory-used 80))
                        (test (<= ?memory-used 89))
                )
=>
                (assert (warning node ?nid memory-used ?memory-used))
)

(defrule node-memory-good
                (t-node (nid ?nid) (memory-used ?memory-used))
                (test (<= ?memory-used 79))
=>
                (assert (good node ?nid memory-used ?memory-used))
)

;********************
;*     SWAP         *
;********************

(defrule node-swap-error
                (t-node (nid ?nid) (swap ?swap))
                (test (= ?swap 100))
=>
                (assert (error node ?nid swap ?swap))
                (assert (update status error))
                (assert (swap 10 minute check))
)

(defrule node-swap-critical
                (t-node (nid ?nid) (swap ?swap))
                (and
                        (test (>= ?swap 90))
                        (test (<= ?swap 99))
                )
=>
                (assert (critical node ?nid swap ?swap))
                (assert (update status critical))
                (assert (swap 10 minute check))
)

(defrule node-swap-warning
                (t-node (nid ?nid) (swap ?swap))
                (and
                        (test (>= ?swap 80))
                        (test (<= ?swap 89))
                )
=>
                (assert (warning node ?nid swap ?swap))
)

(defrule node-swap-good
                (t-node (nid ?nid) (swap ?swap))
                (test (<= ?swap 79))
=>
                (assert (good node ?nid swap ?swap))
)

;************************
;*    DISK-USAGE        *
;************************

(defrule filesystems-du-error
                (t-filesystems (nid ?nid) (fid ?fid) (du-free ?du-free))
                (test (= ?du-free 0))
=>
                (assert (error node ?nid filesystem ?fid du-free ?du-free))
                (assert (update status error))
)

(defrule filesystems-du-critical
                (t-filesystems (nid ?nid) (fid ?fid) (du-free ?du-free))
                (and
                        (test (>= ?du-free 1))
                        (test (<= ?du-free 10))
                )
=>
                (assert (critical node ?nid filesystem ?fid du-free ?du-free))
                (assert (update status critical))
) 

(defrule filesystems-du-warning
                (t-filesystems (nid ?nid) (fid ?fid) (du-free ?du-free))
                (and
                        (test (>= ?du-free 11))
                        (test (<= ?du-free 20))
                )
=>
                (assert (warning node ?nid filesystem ?fid du-free ?du-free))
)

(defrule filesystems-du-good
                (t-filesystems (nid ?nid) (fid ?fid) (du-free ?du-free))
                (test (>= ?du-free 21))
=>
                (assert (good node ?nid filesystem ?fid du-free ?du-free))
)

;************************
;*     READ-TIME        *
;************************

(defrule filesystems-rtime-error
                (t-filesystems (nid ?nid) (fid ?fid) (rtime ?rtime))
                (test (= ?rtime 10))
=>
                (assert (error node ?nid filesystem ?fid rtime ?rtime))
                (assert (update status error))
)

(defrule filesystems-rtime-critical
                (t-filesystems (nid ?nid) (fid ?fid) (rtime ?rtime))
                (and
                        (test (>= ?rtime 7))
                        (test (<= ?rtime 9))
                )
=>
                (assert (critical node ?nid filesystem ?fid rtime ?rtime))
                (assert (update status critical))
)

(defrule filesystems-rtime-warning
                (t-filesystems (nid ?nid) (fid ?fid) (rtime ?rtime))
                (and
                        (test (>= ?rtime 4))
                        (test (<= ?rtime 6))
                )
=>
                (assert (warning node ?nid filesystem ?fid rtime ?rtime))
)

(defrule filesystems-rtime-good
                (t-filesystems (nid ?nid) (fid ?fid) (rtime ?rtime))
                (test (<= ?rtime 3))
=>
                (assert (good node ?nid filesystem ?fid rtime ?rtime))
)

;**********************
;*    WRITE-TIME      *
;**********************

(defrule filesystems-wtime-error
                (t-filesystems (nid ?nid) (fid ?fid) (wtime ?wtime))
                (test (= ?wtime 10))
=>
                (assert (error node ?nid filesystem ?fid wtime ?wtime))
                (assert (update status error))
)

(defrule filesystems-wtime-critical
                (t-filesystems (nid ?nid) (fid ?fid) (wtime ?wtime))
                (and
                        (test (>= ?wtime 7))
                        (test (<= ?wtime 9))
                )
=>
                (assert (critical node ?nid filesystem ?fid wtime ?wtime))
                (assert (update status critical))
)
                
(defrule filesystems-wtime-warning
                (t-filesystems (nid ?nid) (fid ?fid) (wtime ?wtime))
                (and
                        (test (>= ?wtime 4))
                        (test (<= ?wtime 6))
                )
=>
                (assert (warning node ?nid filesystem ?fid wtime ?wtime))
)

(defrule filesystems-wtime-good
                (t-filesystems (nid ?nid) (fid ?fid) (wtime ?wtime))
                (test (<= ?wtime 3))
=>
                (assert (good node ?nid filesystem ?fid wtime ?wtime))
)

;************************
;*      RX-BYTES        *
;************************

(defrule network-rxbytes-error
                (t-network (nid ?nid) (iid ?iid) (rx-bytes ?rx-bytes))
                (test (= ?rx-bytes 10))
=>
                (assert (error node ?nid interface ?iid rx-bytes ?rx-bytes))
                (assert (update status error))
)
                
(defrule network-rxbytes-critical
                (t-network (nid ?nid) (iid ?iid) (rx-bytes ?rx-bytes))
                (and
                        (test (>= ?rx-bytes 7))
                        (test (<= ?rx-bytes 9))
                )
=>
                (assert (critical node ?nid interface ?iid rx-bytes ?rx-bytes))
                (assert (update status critical))
)

(defrule network-rxbytes-warning
                (t-network (nid ?nid) (iid ?iid) (rx-bytes ?rx-bytes))
                (and
                        (test (>= ?rx-bytes 4))
                        (test (<= ?rx-bytes 6))
                )
=>
                (assert (warning node ?nid interface ?iid rx-bytes ?rx-bytes))
)

(defrule network-rxbytes-good
                (t-network (nid ?nid) (iid ?iid) (rx-bytes ?rx-bytes))
                (test (<= ?rx-bytes 3))
=>
                (assert (good node ?nid interface ?iid rx-bytes ?rx-bytes))
)

;********************
;*    TX-BYTES      *
;********************

(defrule network-txbytes-error
                (t-network (nid ?nid) (iid ?iid) (tx-bytes ?tx-bytes))
                (test (= ?tx-bytes 10))
=>
                (assert (error node ?nid interface ?iid tx-bytes ?tx-bytes))
                (assert (update status error))
)
                
(defrule network-txbytes-critical
                (t-network (nid ?nid) (iid ?iid) (tx-bytes ?tx-bytes))
                (and
                        (test (>= ?tx-bytes 7))
                        (test (<= ?tx-bytes 9))
                )
=>
                (assert (critical node ?nid interface ?iid tx-bytes ?tx-bytes))
                (assert (update status critical))
)

(defrule network-txbytes-warning
                (t-network (nid ?nid) (iid ?iid) (tx-bytes ?tx-bytes))
                (and
                        (test (>= ?tx-bytes 4))
                        (test (<= ?tx-bytes 6))
                )
=>
                (assert (warning node ?nid interface ?iid tx-bytes ?tx-bytes))
)

(defrule network-txbytes-good
                (t-network (nid ?nid) (iid ?iid) (tx-bytes ?tx-bytes))
                (test (<= ?tx-bytes 3))
=>
                (assert (good node ?nid interface ?iid tx-bytes ?tx-bytes))
)

;*******************
;*    RX-ERRORS    *
;*******************

(defrule network-rxerrors-error
                (t-network (nid ?nid) (iid ?iid) (rx-errors ?rx-errors))
                (test (= ?rx-errors 10))
=>
                (assert (error node ?nid interface ?iid rx-errors ?rx-errors))
                (assert (update status error))
)

(defrule network-rxerrors-critical
                (t-network (nid ?nid) (iid ?iid) (rx-errors ?rx-errors))
                (and
                        (test (>= ?rx-errors 7))
                        (test (<= ?rx-errors 9))
                )
=>
                (assert (critical node ?nid interface ?iid rx-errors ?rx-errors))
                (assert (update status critical))
)
 
(defrule network-rxerrors-warning
                (t-network (nid ?nid) (iid ?iid) (rx-errors ?rx-errors))
                (and 
                        (test (>= ?rx-errors 4))
                        (test (<= ?rx-errors 6))
                )
=>
                (assert (warning node ?nid interface ?iid rx-errors ?rx-errors))
) 

(defrule network-rxerrors-good
                (t-network (nid ?nid) (iid ?iid) (rx-errors ?rx-errors))
                (test (<= ?rx-errors 3))
=>
                (assert (good node ?nid interface ?iid rx-errors ?rx-errors))
)

;***********************
;*    TX-DROPPED       *
;***********************

(defrule network-txdropped-error
                (t-network (nid ?nid) (iid ?iid) (tx-dropped ?tx-dropped))
                (test (= ?tx-dropped 10))
=>
                (assert (error node ?nid interface ?iid tx-dropped ?tx-dropped))
                (assert (update status error))
)

(defrule network-txdropped-critical
                (t-network (nid ?nid) (iid ?iid) (tx-dropped ?tx-dropped))
                (and
                        (test (>= ?tx-dropped 7))
                        (test (<= ?tx-dropped 9))
                )
=>
                (assert (critical node ?nid interface ?iid tx-dropped ?tx-dropped))
                (assert (update status critical))
)

(defrule network-txdropped-warning
                (t-network (nid ?nid) (iid ?iid) (tx-dropped ?tx-dropped))
                (and
                        (test (>= ?tx-dropped 4))
                        (test (<= ?tx-dropped 6))
                )
=>
                (assert (warning node ?nid interface ?iid tx-dropped ?tx-dropped))
) 
                
(defrule network-txdropped-good
                (t-network (nid ?nid) (iid ?iid) (tx-dropped ?tx-dropped))
                (test (<= ?tx-dropped 3))
=>
                (assert (good node ?nid interface ?iid tx-dropped ?tx-dropped))
)

;********************************
;             5 MIN             *
;********************************

;****************************
;*         LOADAVG          *
;****************************

(defrule node-loadavg-error-five
                (loadavg 5 minute check)
                ?ldvg <- (loadavg 5 minute check)
                (t-node-five (nid ?nid) (loadavg-five ?loadavg-five))
                (test (= ?loadavg-five 300))
=>
                (assert (error node ?nid loadavg-five ?loadavg-five))
                (assert (loadavg 10 minute check))
                (retract ?ldvg)
                (assert (update status error five))
)

(defrule node-loadavg-critical-five
                (loadavg 5 minute check)
                ?ldvg <- (loadavg 5 minute check)
                (t-node-five (nid ?nid) (loadavg-five ?loadavg-five))
                (and
                        (test (>= ?loadavg-five 200))
                        (test (<= ?loadavg-five 299))
                )
=>
                (assert (critical node ?nid loadavg-five ?loadavg-five))
                (assert (loadavg 10 minute check))
                (retract ?ldvg)
                (assert (update status critical five))
)

(defrule node-loadavg-warning-five
                (loadavg 5 minute check)
                ?ldvg <- (loadavg 5 minute check)
                (t-node-five (nid ?nid) (loadavg-five ?loadavg-five))
                (and
                        (test (>= ?loadavg-five 100))
                        (test (<= ?loadavg-five 199))
                )
=>
                (assert (warning node ?nid loadavg-five ?loadavg-five))
                (retract ?ldvg)
)

(defrule node-loadavg-good-five
                (loadavg 5 minute check)
                ?ldvg <- (loadavg 5 minute check)
                (t-node-five (nid ?nid) (loadavg-five ?loadavg-five))
                (test (<= ?loadavg-five 99))
=>
                (assert (good node ?nid loadavg-five ?loadavg-five))
                (retract ?ldvg)
)

;**************************
;*        MEMORY          *
;**************************

(defrule node-memory-error-five
                (memory 5 minute check)
                ?ldvg <- (memory 5 minute check)
                (t-node-five (nid ?nid) (memory-used-five ?memory-used-five))
                (test (= ?memory-used-five 100))
=>
                (assert (error node ?nid memory-used-five ?memory-used-five))
                (assert (update status error five))
                (assert (memory 10 minute check))
                (retract ?ldvg)
)

(defrule node-memory-critical-five
                (memory 5 minute check)
                ?ldvg <- (memory 5 minute check)
                (t-node-five (nid ?nid) (memory-used-five ?memory-used-five))
                (and
                        (test (>= ?memory-used-five 90))
                        (test (<= ?memory-used-five 99))
                )
=>
                (assert (critical node ?nid memory-used-five ?memory-used-five))
                (assert (update status critical five))
                (assert (memory 10 minute check))
                (retract ?ldvg)
)

(defrule node-memory-warning-five
                (memory 5 minute check)
                ?ldvg <- (memory 5 minute check)
                (t-node-five (nid ?nid) (memory-used-five ?memory-used-five))
                (and
                        (test (>= ?memory-used-five 80))
                        (test (<= ?memory-used-five 89))
                )
=>
                (assert (warning node ?nid memory-used-five ?memory-used-five))
                (retract ?ldvg)
)

(defrule node-memory-good-five
                (memory 5 minute check)
                ?ldvg <- (memory 5 minute check)
                (t-node-five (nid ?nid) (memory-used-five ?memory-used-five))
                (test (<= ?memory-used-five 79))
=>
                (assert (good node ?nid memory-used-five ?memory-used-five))
                (retract ?ldvg)
)

;********************
;*     SWAP         *
;********************

(defrule node-swap-error-five
                (swap 5 minute check)
                ?ldvg <- (swap 5 minute check)
                (t-node-five (nid ?nid) (swap-five ?swap-five))
                (test (= ?swap-five 100))
=>
                (assert (error node ?nid swap-five ?swap-five))
                (assert (update status error five))
                (assert (swap 10 minute check))
                (retract ?ldvg)
)

(defrule node-swap-critical-five
                (swap 5 minute check)
                ?ldvg <- (swap 5 minute check)
                (t-node-five (nid ?nid) (swap-five ?swap-five))
                (and
                        (test (>= ?swap-five 90))
                        (test (<= ?swap-five 99))
                )
=>
                (assert (critical node ?nid swap-five ?swap-five))
                (assert (update status critical five))
                (assert (swap 10 minute check))
                (retract ?ldvg)
)

(defrule node-swap-warning-five
                (swap 5 minute check)
                ?ldvg <- (swap 5 minute check)
                (t-node-five (nid ?nid) (swap-five ?swap-five))
                (and
                        (test (>= ?swap-five 80))
                        (test (<= ?swap-five 89))
                )
=>
                (assert (warning node ?nid swap-five ?swap-five))
                (retract ?ldvg)
)

(defrule node-swap-good-five
                (swap 5 minute check)
                ?ldvg <- (swap 5 minute check)
                (t-node-five (nid ?nid) (swap-five ?swap-five))
                (test (<= ?swap-five 79))
=>
                (assert (good node ?nid swap-five ?swap-five))
                (retract ?ldvg)
)

;********************************
;            10 MIN             *
;********************************

;****************************
;*         LOADAVG          *
;****************************

(defrule node-loadavg-error-ten
                (loadavg 10 minute check)
                ?ldvg <- (loadavg 10 minute check)
                (t-node-ten (nid ?nid) (loadavg-ten ?loadavg-ten))
                (test (= ?loadavg-ten 300))
=>
                (assert (error node ?nid loadavg-ten ?loadavg-ten))
                (retract ?ldvg)
                (assert (update status error ten))
)

(defrule node-loadavg-critical-ten
                (loadavg 10 minute check)
                ?ldvg <- (loadavg 10 minute check)
                (t-node-ten (nid ?nid) (loadavg-ten ?loadavg-ten))
                (and
                        (test (>= ?loadavg-ten 200))
                        (test (<= ?loadavg-ten 299))
                )
=>
                (assert (critical node ?nid loadavg-ten ?loadavg-ten))
                (retract ?ldvg)
                (assert (update status critical ten))
)

(defrule node-loadavg-warning-ten
                (loadavg 10 minute check)
                ?ldvg <- (loadavg 10 minute check)
                (t-node-ten (nid ?nid) (loadavg-ten ?loadavg-ten))
                (and
                        (test (>= ?loadavg-ten 100))
                        (test (<= ?loadavg-ten 199))
                )
=>
                (assert (warning node ?nid loadavg-ten ?loadavg-ten))
                (retract ?ldvg)
)

(defrule node-loadavg-good-ten
                (loadavg 10 minute check)
                ?ldvg <- (loadavg 10 minute check)
                (t-node-ten (nid ?nid) (loadavg-ten ?loadavg-ten))
                (test (<= ?loadavg-ten 99))
=>
                (assert (good node ?nid loadavg-ten ?loadavg-ten))
                (retract ?ldvg)
)

;**************************
;*        MEMORY          *
;**************************

(defrule node-memory-error-ten
                (memory 10 minute check)
                ?ldvg <- (memory 10 minute check)
                (t-node-ten (nid ?nid) (memory-used-ten ?memory-used-ten))
                (test (= ?memory-used-ten 100))
=>
                (assert (error node ?nid memory-used-ten ?memory-used-ten))
                (assert (update status error ten))
                (retract ?ldvg)
)

(defrule node-memory-critical-ten
                (memory 10 minute check)
                ?ldvg <- (memory 10 minute check)
                (t-node-ten (nid ?nid) (memory-used-ten ?memory-used-ten))
                (and
                        (test (>= ?memory-used-ten 90))
                        (test (<= ?memory-used-ten 99))
                )
=>
                (assert (critical node ?nid memory-used-ten ?memory-used-ten))
                (assert (update status critical ten))
                (retract ?ldvg)
)

(defrule node-memory-warning-ten
                (memory 10 minute check)
                ?ldvg <- (memory 10 minute check)
                (t-node-ten (nid ?nid) (memory-used-ten ?memory-used-ten))
                (and
                        (test (>= ?memory-used-ten 80))
                        (test (<= ?memory-used-ten 89))
                )
=>
                (assert (warning node ?nid memory-used-ten ?memory-used-ten))
                (retract ?ldvg)
)

(defrule node-memory-good-ten
                (memory 10 minute check)
                ?ldvg <- (memory 10 minute check)
                (t-node-ten (nid ?nid) (memory-used-ten ?memory-used-ten))
                (test (<= ?memory-used-ten 79))
=>
                (assert (good node ?nid memory-used-ten ?memory-used-ten))
                (retract ?ldvg)
)

;********************
;*     SWAP         *
;********************

(defrule node-swap-error-ten
                (swap 10 minute check)
                ?ldvg <- (swap 10 minute check)
                (t-node-ten (nid ?nid) (swap-ten ?swap-ten))
                (test (= ?swap-ten 100))
=>
                (assert (error node ?nid swap-ten ?swap-ten))
                (assert (update status error ten))
                (retract ?ldvg)
)

(defrule node-swap-critical-ten
                (swap 10 minute check)
                ?ldvg <- (swap 10 minute check)
                (t-node-ten (nid ?nid) (swap-ten ?swap-ten))
                (and
                        (test (>= ?swap-ten 90))
                        (test (<= ?swap-ten 99))
                )
=>
                (assert (critical node ?nid swap-ten ?swap-ten))
                (assert (update status critical ten))
                (retract ?ldvg)
)

(defrule node-swap-warning-ten
                (swap 10 minute check)
                ?ldvg <- (swap 10 minute check)
                (t-node-ten (nid ?nid) (swap-ten ?swap-ten))
                (and
                        (test (>= ?swap-ten 80))
                        (test (<= ?swap-ten 89))
                )
=>
                (assert (warning node ?nid swap-ten ?swap-ten))
                (retract ?ldvg)
)

(defrule node-swap-good-ten
                (swap 10 minute check)
                ?ldvg <- (swap 10 minute check)
                (t-node-ten (nid ?nid) (swap-ten ?swap-ten))
                (test (<= ?swap-ten 79))
=>
                (assert (good node ?nid swap-ten ?swap-ten))
                (retract ?ldvg)
)

