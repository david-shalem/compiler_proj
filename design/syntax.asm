;variables
int
float
char

<type> <name> = <value>

<name> = (<type>)<name>

;function
<type> <name>(<param type> <param name>, ...) {
code....
return <name>
}

;raw assembly in the code
raw{
 assembly code...
}           
    
;math operators            
<name> +\-\*\%\/\~\|\&\^\<<\>> <name> 

;system functions

sleep(<time (ms)>) ;INT 15h / AH = 86h
date() ;INT 21h / AH=2Ah        
time() ;INT 21h / AH=2Ch
print(<string (with '$' at the end)>) ;INT 21h / AH=9 
read() ;INT 21h / AH=0Ah ;need to clean the buffer after it
exit() ;INT 21h / AH=4Ch  
rand(<limit>) ;INT 1Ah / AH = 00h 


;structs
struct <struct name>{
<param1 type> <param1 name>
<param2 type> <param2 name>
...

}


<struct name> <name> (<param1 name> = <value>, <param2 name> = <value>...)

;lops
while(<condition>){
code...
}

;conidtions
if(condition) {
code...
}
else if(condition){
code..
}
else {
code...
}      

















