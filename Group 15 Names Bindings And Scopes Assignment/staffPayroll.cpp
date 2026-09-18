#include <iostream>
#include <string>
#include <vector>
#include <limits>

//A) Typed payroll model
//=====================================================================================================================================
//Global variables
const double TRAINING_BAND_LIMIT = 50000.0;
const double TRAINING_RATE_LOW = 0.10;
const double TRAINING_RATE_HIGH = 0.15;
const double OTHER_DEDUCTIONS = 500.0;
const int MIN_STAFF_RECORDS = 5;

//A struct for the record of each staff member
struct StaffRecord {
    std::string staffId;
    std::string name;
    double      basicPay;
    double      houseAllowance;
    double      transportAllowance;
    bool        isActive;
};

//B) Functions and parameter scope
//=====================================================================================================================================

double computeGrossPay(const StaffRecord& r) {
    double gross = r.basicPay + r.houseAllowance + r.transportAllowance;
    return gross;
}

/*Double gross is a local variable 
whose scope is the body of gross pay only
It's life time is stack dynamic as it is created when the function is called
destroyed when the function returns*/

double computeTrainingDeduction(double gross){
    double deduction;
    if (gross <= TRAINING_BAND_LIMIT) {
        deduction = gross * TRAINING_RATE_LOW;
    } else {
        double excess = gross - TRAINING_BAND_LIMIT;
        deduction = (TRAINING_BAND_LIMIT * TRAINING_RATE_LOW) + (excess * TRAINING_RATE_HIGH);
    }
    return deduction;
}

/*In the function above the parameter here is also called gross but it is a completely different variable from the gross local inside gross pay.
This is because c++ uses static (lexical) scoping where the compiler resolves each gross by where it is written, not by any runtime link between the two functions.
They have the same spelling but are unrelated variables.*/

/*  
inside the else { } — this is block-scoped: it 
only exists between that pair of braces. If you try using 
excess after the else block ends  the compiler
 will reject it.
*/

double computeNetPay(double gross, double training) {
    return gross - training - OTHER_DEDUCTIONS;
}

//PART D: REFERENCE/OUTPUT PARAMETER
//=====================================================================================================================================
/*This function calculates BOTH gross pay and training deduction in one
call, and sends both results back to the caller through reference
parameters (outGross, outDeduction) instead of a single return value.*/

void computeGrossAndDeduction(const StaffRecord& r, double& outGross, double& outDeduction) {
    outGross = computeGrossPay(r);            // writes directly into the caller's variable
    outDeduction = computeTrainingDeduction(outGross); // writes directly into the caller's variable
}

/*THE EXPLANATION OF ALIASING:
When main() calls the function 'computeGrossAndDeduction(r, gross, training);',
the parameters outGross and outDeduction become ALIASES for main()'s
gross and training variables .This means that outGross and gross are two
different NAMES referring to the exact same memory location for the
duration of this call. When the line 'outGross = computeGrossPay(r);'
runs, it is not copying a value back afterward ,but it is writing straight
into main()'s gross variable, because they are literally the same
object. This is how one C++ function can hand back two results without
using two separate return statements.*/

void printPayslip(const StaffRecord& r, double gross, double training, double net) {

    
    //PART C: USING A STATIC LOCAL VARIABLE
    //=====================================================================================================================================
    static int payslipCount = 0; // static variable to keep track of the number of payslips printed
    payslipCount++; // increment the count each time a payslip is printed
    std::cout << "\n===========PAYSLIP============\n"; 
    std::cout << "Payslip for : " << r.name << " (" << r.staffId << ") ----\n";
    std::cout << "Gross Pay:          " << gross << "\n";
    std::cout << "Training Deduction: " << training << "\n";
    std::cout << "Other Deduction:    " << OTHER_DEDUCTIONS << "\n";
    std::cout << "Net Pay:            " << net << "\n\n";
}



//INPUT FUNCTIONS — VALIDATED MANUAL ENTRY OF STAFF RECORDS
//=====================================================================================================================================

//Reads one full line of text and keeps asking until it is not empty.
//Handles exceptional case1: blank name/ID input.
std::string getValidatedString(const std::string& prompt) {
    std::string value;
    while (true) {
        std::cout << prompt;
        std::getline(std::cin, value);
        if (!value.empty()) {
            return value;
        }
        std::cout << "  Input cannot be empty. Please try again.\n";
    }
}

/*Reads a number and keeps asking until it is a valid double that is not
negative. Handles exceptional case #2: non-numeric text typed where a
number was expected, and exceptional case #3: a negative pay/allowance
value, which is not realistic for this payroll case.*/

double getValidatedDouble(const std::string& prompt, double minValue = 0.0) {
    double value;
    while (true) {
        std::cout << prompt;
        std::cin >> value;

        if (std::cin.fail() || value < minValue) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "  Invalid input. Please enter a number >= " << minValue << ".\n";
            continue;
        }

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return value;
    }
}

//Reads a y/n answer and keeps asking until it gets one.
bool getValidatedYesNo(const std::string& prompt) {
    std::string answer;
    while (true) {
        std::cout << prompt;
        std::getline(std::cin, answer);
        if (answer == "y" || answer == "Y") return true;
        if (answer == "n" || answer == "N") return false;
        std::cout << "  Please answer y or n.\n";
    }
}

//Builds ONE StaffRecord from keyboard input using the validators above.
StaffRecord inputStaffRecord(int recordNumber) {
    StaffRecord r;

    std::cout << "\n--- Enter details for staff member " << recordNumber << " ---\n";
    r.staffId             = getValidatedString("Staff ID: ");
    r.name                = getValidatedString("Full name: ");
    r.basicPay             = getValidatedDouble("Basic pay: ", 0.0);
    r.houseAllowance       = getValidatedDouble("House allowance: ", 0.0);
    r.transportAllowance   = getValidatedDouble("Transport allowance: ", 0.0);
    r.isActive             = getValidatedYesNo("Is this staff member active? (y/n): ");

    return r;
}

/*Repeats inputStaffRecord() until MIN_STAFF_RECORDS is reached, then asks
the user whether to keep adding more.*/
std::vector<StaffRecord> inputAllStaffRecords() {
    std::vector<StaffRecord> staff;
    int count = 0;

    while (true) {
        ++count;
        staff.push_back(inputStaffRecord(count));

        if (static_cast<int>(staff.size()) < MIN_STAFF_RECORDS) {
            std::cout << "(" << staff.size() << "/" << MIN_STAFF_RECORDS
                      << " minimum records entered -- please continue)\n";
            continue;
        }

        bool addAnother = getValidatedYesNo("Do you want to add another staff record? (y/n): ");
        if (!addAnother) {
            break;
        }
        
    }

    return staff;
}


int main() {
  

    std::cout << "=== Staff Payroll and Allowance Processing Simulator ===\n\n";

    std::vector<StaffRecord> staff = inputAllStaffRecords();

    double sumGross = 0.0;
   double sumDeductions = 0.0;
   double sumNet = 0.0;
   double highestNet = -1.0;
   int totalPayslipsPrinted = 0;

    for (const auto& r : staff) {
        double gross = 0.0; //gross = computeGrossPay(r);
        double training = 0.0; //training =  computeTrainingDeduction(gross);
        computeGrossAndDeduction(r, gross, training); // This demonstrates reference parameters
        

        /* TASK A/B: TYPE INFERENCE USING 'auto'
        'auto' infers net's type as double based on computeNetPay's return type at compile time.
        */
        auto net = computeNetPay(gross, training);
        auto totalDeduction = training + OTHER_DEDUCTIONS;
        
        sumGross += gross;
        sumDeductions += totalDeduction;
        sumNet += net;
    if (net > highestNet) {
        highestNet = net;
    }
       ++totalPayslipsPrinted;
       printPayslip(r, gross, training, net);

        //PART C: THE CONCEPT OF SHADOWING
        //====================================================================================================================================
        double basicpay = r.basicPay; //declares a new variable basicpay that shadows the member variable r.basicPay
        std:: cout << "====THE CONCEPT OF SHADOWING====\n";
        std::cout << "Outer basic pay before introducing a block = " << r.basicPay << "\n";
       
        {
            double basicpay = r.basicPay * 0.60; //delibarately using this name to shadow the outer basicpay variable
            std::cout << "Inner basic pay after introducing a block = " << basicpay << "\n";
        }

        std::cout << "Outer basic pay after the execution of the block = " << basicpay << "\n";
    }

       
        std::cout << " ======== PAYROLL SUMMARY REPORT============\n";
        std::cout << "Total Gross Pay:       " << sumGross << "\n";
        std::cout << "Total Deductions:      " << sumDeductions << "\n";
        std::cout << "Total Net Pay:         " << sumNet << "\n";
        std::cout << "Highest Net Pay:       " << highestNet << "\n";
        std::cout << "Total Payslips Issued: " << totalPayslipsPrinted << "\n";
        std::cout << "=======================================================\n";

    return 0;
  /*PART E :EXPLANATION OF WHY STATIC TYPE BINDING CATCHES ERRORS EARLY
Every variable and parameter in this program (StaffRecord's fields,
function parameters, return types) is bound to a fixed type at compile
time. This lets the compiler check every assignment and function call
BEFORE the program ever runs. For example, if we accidentally wrote
r.basicPay = r.name; the compiler would refuse to build the program at
all, because a std::string cannot be assigned to a double. In a
dynamically typed language, this mistake would only be discovered when
that exact line executes at runtime , possibly after payroll had
already been partly processed for other staff. Static type binding
therefore converts many potential runtime bugs into compile-time
errors that must be fixed before the program can even be tested.*/

}