# Smart-canteen-billing-system
...


Smart Canteen Billing System (C)
A console-based college project for managing canteen food orders, billing, stock, customer wallets, employee allowances, loyalty rewards, and sales reports. The project is written in C and uses local .dat files for persistence.
Features
Student
- Register and log in with a student ID and password.
- Browse the food menu, place orders, and receive itemized bills with order IDs.
- View order history and check an order's status.
- Cancel an eligible pending order and receive a wallet refund when the original payment was made by wallet.
- Check wallet balance and add money through simulated Cash/UPI top-ups.
- Earn loyalty rewards after an order is marked Completed: 1 point per full Rs.100 of discounted food spending; each point immediately credits Rs.10 to the student wallet.
Employee
- Log in with an employee ID and password.
- Place food orders with an additional 5% employee discount after the order-value discount.
- Use two separate wallets:
  - Monthly allowance: Rs.3,000, refreshed when a new month is detected; unused allowance does not carry over.
  - Personal wallet: stores employee top-ups and loyalty rewards; balance carries over between months.
- Wallet purchases spend the monthly allowance first, then the personal wallet.
- Add money to the personal wallet through simulated Cash/UPI top-ups.
- Earn the same loyalty rewards as students after eligible orders are completed.
- View inventory, add food items, update stock quantities and food prices, see low-stock warnings, view orders, and update order status.
Admin
- Log in and manage food items (add, delete/deactivate, and update details).
- Add or remove/deactivate employee accounts and view employees.
- View all orders and update order statuses.
- View daily and monthly sales, food costs, gross profit/loss, and environmental donation totals.
- Historical orders remain available when an item or employee is deactivated.
Guest (no account)
- Enter a name and phone number for the current session and order without logging in.
- Guest name and phone number are not stored in the project data files.
- Receive an order ID and use the main-menu option to cancel an eligible pending guest order by ID.
- Guest orders receive order-value discounts but not employee discounts or loyalty rewards.
Discounts and billing
Order-value discounts apply to the food subtotal. The single highest qualifying tier is used:
Food subtotal	Order discount
Rs.500 or less	0%
Above Rs.500 up to Rs.1,000	5%
Above Rs.1,000 up to Rs.1,500	10%
Above Rs.1,500 up to Rs.2,000	15%
Above Rs.2,000	20%


Employees receive an additional 5% discount on the food amount after the tier discount. At checkout, every customer may optionally add Rs.5 for environmental protection NGOs. The donation is added after discounts, does not earn loyalty points, and is reported separately from food sales.

Example: Employee food subtotal Rs.2,500 -> 20% tier discount (Rs.500) -> 5% employee discount on Rs.2,000 (Rs.100) -> food payable Rs.1,900; with optional donation, total Rs.1,905.
Order status and loyalty
- Orders start as Pending.
- Employees and admins can change eligible orders to Preparing or Completed.
- Eligible pending orders may be cancelled; cancelled stock is restored.
- Loyalty points are awarded once, when a student or employee order is marked Completed.
- Points are calculated using whole Rs.100 increments of discounted food spending, excluding NGO donations. Each point is instantly converted to Rs.10 in the relevant personal/student wallet.
How to compile and run
Requirements: a C compiler such as GCC; no external libraries or internet connection needed.
gcc smart_canteen.c -o smart_canteen
./smart_canteen
On Windows with GCC:
gcc smart_canteen.c -o smart_canteen.exe
smart_canteen.exe
Run the executable from the project folder so the .dat files are created and read there.
Default demo credentials
Role	Login
Admin	Password: admin123
Employee	ID: 201, password: emp123
Student	Choose Student Registration to create an account.
Student id = 0078, pass = 0078
Guest	No login; provide a name and phone number for the current order.


These credentials are for demonstration only; passwords are not secured for real-world use.
Main menu
1. Student Login
2. Student Registration
3. Employee Login
4. Admin Login
5. Guest Order (No Login)
6. Cancel Guest Order by ID
7. Exit
Data storage
The program uses binary .dat files for food items, registered students and employees, orders, wallet balances/debits, and loyalty reward tracking. Guest contact details are temporary local variables and are not saved. Keep existing .dat files if you want to preserve your project data when recompiling.
Important limitations
- This is an educational demonstration, not a production payment or banking system.
- Cash/UPI top-ups and payments are simulated; no real transaction or NGO transfer is performed.
- Profit/loss reporting represents gross food profit (net food sales minus recorded food costs), not full net business profit after rent, wages, utilities, etc.
- Guest cancellation by order ID alone is convenient for a demo but would require stronger verification in a real deployment.
- Employee personal-wallet top-ups and loyalty rewards persist across monthly allowance resets. Existing historical mixed-wallet balances cannot be reliably split retroactively.
Technologies used
C language, structures (struct), arrays, functions, loops, conditional statements, switch, standard input/output, time functions, and binary file handling.
