(deftemplate node-data
		(slot id)
		(slot memory)
		(slot cpu))


(deffacts nodes
		(node-data (id 1) (memory 300) (cpu 50))
		(node-data (id 2) (memory 100) (cpu 15))
		(node-data (id 3) (memory 400) (cpu 10))
)

(reset)

(defrule is-free
		(logical (node (id ?id) (memory ?memory) (cpu ?cpu)))
		(logical (test (> ?memory 200)))
		(logical (test (> ?cpu 30)))
=>
		(assert is-free ?id)
		(printout t ?id " is free." crlf)
)
