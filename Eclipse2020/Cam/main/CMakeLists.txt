idf_component_register(SRCS			 "main.c"
									  "./components/wifi/wifi.c" 
									  "./components/camera/camera.c" 
									  "./components/camera/bitmap.c"
									  "./components/camera/ov2640.c"
									  "./components/camera/sccb.c"
									  "./components/camera/twi.c"
									  "./components/camera/wiring.c"
									  "./components/camera/xclk.c"
									  "./components/http_server/my_http_server.c"
									  "./components/quirc/decode.c"
									  "./components/quirc/identify.c"
									  "./components/quirc/quirc.c"
									  "./components/quirc/version_db.c"
									  "./qr_recoginize.c"
									  "./led.c"
									  
									  
				      INCLUDE_DIRS   "." 
									  "./components/quirc" 
									  "./components/http_server"
									  "./components/camera"
									  
					  )

