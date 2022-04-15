idf_component_register(SRCS			 "main.c" 
									  "./components/Station/Station.c" 
									  "./components/StorageC/StorageC.c"
									  "./components/GPS/gps.c"
									  "./components/Pulse/Pulse.c"
									  "./components/LCD_20x4/LCD_20x4.c"
									  "./params.c"
									  "./str.c"
									  
									  
				      INCLUDE_DIRS   "." 
				      				 "./components/StorageC"
				      				 "./components/Station"
				      				 "./components/GPS"
				      				 "./components/Pulse"
				      				 "./components/LCD_20x4"
					  )

