#!/bin/gawk
# @file HWC12.awk
# @author Graham Patterson
# @brief CESIL interpreter
#
#  @details Batch mode interpreter for CESIL language.
#  CESIL tends to use the capabilities of the underlying machine
#  so integers are typically 32-bit or 64-bit.
#
# @note As things stand, address labels and the labels for the storage use a common repository.
# so it is possible to STORE label JUMP label to calculate a new
# instruction location and transfer control. Yea! self modifying code!
#

## 
# @brief Setup data and counters
#
BEGIN   {
            if(debug == "" || debug == 0) {
                debug = 0
            } else {
                debug = 1; printf("Debug output enabled\n")
            }
            
            accumulator = 0; havedata = 0 ; endofdata = 0
            programcounter = 1 # instruction offset
            datacounter    = 1 # data (embedded) offset
            ip             = 1 # current program instruction offset
            dc             = 1 # current data value offset
            recno = 0; overrun = 5000
            opnames["IN"]     = 0
            opnames["OUT"]    = 0
            opnames["PRINT"]  = 1
            opnames["LINE"]   = 0
            opnames["LOAD"]   = 1
            opnames["STORE"]  = 1
            opnames["ADD"]    = 1
            opnames["SUBTRACT"] = 1
            opnames["MULTIPLY"] = 1
            opnames["DIVIDE"] = 1
            opnames["JUMP"]   = 1
            opnames["JINEG"]  = 1
            opnames["JIZERO"] = 1
            opnames["HALT"]   = 0
            printf("CESIL Interpreter, Graham Patterson 2022\nLoading...\n")
        }

/^[\t ]*$/ || /^[\t ]*\(/ || /^[\t ]*[*]/    {
                programcounter++
                next
            }
            
/^[\t ]*%/    {
                havedata = 1
                # read line and store until * end of data mark
                while(getline > 0 && $0 !~ /^[ ]*\*/) {
                    data[datacounter++] = $1
                }
                endofdata = 1
                next
            }

!isoperator($1)  {
                labels[programcounter] = $1
                labelsbyname[$1] = programcounter
                $1 = "" # Remove the label
                $0 = $0                
            }

# anything not specified - program instructions 
        {
            operator = $1
            operand  = $2
            if(isoperator($1))
            {
                if(NF > 2) {
                    for(i = 3; i <= NF; i++) {
                        operand = operand " " $i
                    }
                }
                operators[programcounter] = operator
                operands[programcounter] = operand
                programcounter++
           }
        }
        
        
END {
        printf("Checking...\n")
        for(var in operands) { 
            if(isoperator(operands[var])) {
                printf("Warning: Operand '%s' is the name of an operator.\n", operands[var])
            }
        } # end for
        for(var in labelsbyname) { 
            if(isoperator(labelsbyname[var])) {
                printf("Warning: Label '%s' is the name of an operator.\n", labelsbyname[var])
            }
        } # end for
        printf("Running...\n\n")
        n = 0
        while(ip > 0 && ip <= programcounter) {
            ip = execute(ip)
            n++; if(n > overrun) { printf("Over-run limit (%d instructions) reached.\n", overrun) ; break; }
        } # end while
        printf("\nEnd run\n")
        if(debug) {
            printf("\nProgram section\n")
            for(i = 1; i < programcounter; i++) {
                printf("%3d %-20s %-20s %-20s\n", i, labels[i], operators[i], operands[i])
            }
            if(datacounter > 1)
            {
                printf("\nData section\n")
                for(i = 1; i < datacounter; i++) {
                    printf("%3d %-s\n", i, data[i])
                }
            }
            printf("\nLabels and storage\n")
            for(i in labelsbyname) {
                printf("%20s %20s\n", i, labelsbyname[i])
            }
            printf("\nExecution order\n")
            for(i = 1; i < recno; i++) {
                printf("%s", executionrecord[i])
            }
            printf("\nTotal instructions executed: %d\n", n)
        } # end debug
    }

## 
# @brief Test to see if operator is in the operator list.
#
function isoperator(op,    result)
{               
    if(op in opnames) {
        result = 1
    } else {
        #printf("isoperator(): Unrecognised operator '%s', [%s]\n", op, opnames[op])
        result = 0
    }
    return result
}

## 
# @brief Execute one instruction at program offset given by ip.
#  @param ip Current instruction pointer.
function execute(ip,   newip) {
    operator = operators[ip]
    operand = operands[ip]
    if(opnames[operator] == 1 && operand == "") {
        printf("Warning: Operator %s at IP %d expects %d operand.\n", operator, ip, opnames[operator])
    }
    if(debug) {
        executionrecord[recno++] = sprintf("Entry: ip = %3d Operator = %8s Operand = %20s %4s  Acc. %4s\n", ip, operator, operand, labelsbyname[operand], accumulator)
    }
    newip = ip + 1 # default instruction increment
    switch(operator) {
        case "IN":
            if(havedata > 0 && dc < datacounter) {
                accumulator = data[dc++]
            } else { 
              if(!((getline accumulator < "/dev/stdin") > 0))
              { printf("execute(): Unable to read stdin at IP = %d\n", ip) }
            }
            break
        case "OUT":
            printf("%d", accumulator)
            break
        case "PRINT":
            gsub(/^\"/, "", operand)
            gsub(/\"$/, "", operand)
            printf("%s", operand)
            break
        case "LINE":
            printf("\n")
            break
        case "LOAD":
            accumulator = islabel(operand) ? labelsbyname[operand] : operand
            break;
        case "STORE":
            labelsbyname[operand] = accumulator
            break
        case "ADD":
            accumulator += islabel(operand) ?  labelsbyname[operand] : operand
            break
        case "SUBTRACT":
            accumulator -= islabel(operand) ?  labelsbyname[operand] : operand
            break
        case "MULTIPLY":
            accumulator *= islabel(operand) ?  labelsbyname[operand] : operand
            break
        case "DIVIDE":
            temp = islabel(operand) ?  labelsbyname[operand] : operand
            accumulator = int(accumulator / temp)
            break
        case "JUMP":
            newip = labelsbyname[operand]
            break
        case "JINEG":
            newip = (accumulator < 0) ? labelsbyname[operand] : (ip + 1)
            break;
        case "JIZERO":
            newip = (accumulator == 0) ? labelsbyname[operand] : (ip + 1)
            break;
        case "HALT":
             newip = -1
             break
        default: # Treat as a NOP
            newip = ip + 1
            result = 0
            break
    }
    if(debug) {
        executionrecord[recno++] = sprintf("Exit : ip = %3d Operator = %8s Operand = %20s %4s  Acc. %4s\n", ip, operator, operand, labelsbyname[operand], accumulator)
        #printf("%s", executionrecord[recno - 1])
    }
    return newip
}

# If the label was created when the program was read, it will be a programcounter value


##
# @brief Test to see if operand is a label
#
function islabel(operand,    result) {
   result = 0
   if(labelsbyname[operand] != "") {
        result = 1
   }
   return result
}

# end of file
