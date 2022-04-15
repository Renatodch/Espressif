idf_component_register(SRCS			 "main.c" 
									  "./components/src/Station.c" 
									  "./components/src/StorageC.c"
									  "./components/src/gps.c"
									  "./components/src/Pulse.c"
									  "./components/src/Led.c"
									  "./components/src/bt.c"
									  "./params.c"
									  "./str.c"
									  
									  
				      INCLUDE_DIRS   "." 
				      				 "./components/inc"
				      				
					  )

