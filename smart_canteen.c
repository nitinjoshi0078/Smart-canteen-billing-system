/*admin pass = admin123
employee id = 201 , pass = emp123
student id = 0078,pass = 0078
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_ITEMS 100
#define MAX_EMPLOYEES 100
#define MAX_ORDERS 2000
#define MAX_LINES 20
#define MAX_STUDENTS 200
#define WALLET_ALLOWANCE 3000.0
#define DONATION 5.0

/* This is a college demo. Payments and NGO transfers are simulated. */
typedef struct
{
    int id;
    char name[50];
    double price, cost;
    int stock, active;
} Item;
typedef struct
{
    int id;
    char name[50], password[30];
    double wallet;
    int wallet_month, active;
} Employee;
typedef struct
{
    int id;
    char name[50], password[30];
    double wallet;
    int active;
} Student;
typedef struct
{
    int item_id, quantity;
    char name[50];
    double price, cost;
} OrderLine;
typedef struct
{
    int id, customer_id, customer_type, count, status, paid, payment_method;
    char date[11];
    OrderLine lines[MAX_LINES];
    double subtotal, discount, donation, total;
} Order;

Item items[MAX_ITEMS];
Employee employees[MAX_EMPLOYEES];
Student students[MAX_STUDENTS];
Order orders[MAX_ORDERS];
int item_count = 0, employee_count = 0, student_count = 0, order_count = 0;
/* Separate reward ledger preserves compatibility with existing .dat records. */
int rewarded_ids[MAX_ORDERS], rewarded_count = 0;
/* Employee allowance stays in Employee.wallet (legacy file format).
   Personal funds are stored separately so old employees.dat remains readable. */
double personal_wallet[MAX_EMPLOYEES];
typedef struct
{
    int order_id;
    double allowance_used, personal_used;
} WalletDebit;
WalletDebit debits[MAX_ORDERS];
int debit_count = 0;
void save_wallet_details(void)
{
    FILE *f = fopen("employee_personal.dat", "wb");
    int i;
    if (f)
    {
        for (i = 0; i < employee_count; i++)
        {
            fwrite(&employees[i].id, sizeof(int), 1, f);
            fwrite(&personal_wallet[i], sizeof(double), 1, f);
        }
        fclose(f);
    }
    f = fopen("wallet_debits.dat", "wb");
    if (f)
    {
        fwrite(&debit_count, sizeof(int), 1, f);
        fwrite(debits, sizeof(WalletDebit), debit_count, f);
        fclose(f);
    }
}
void load_wallet_details(void)
{
    FILE *f = fopen("employee_personal.dat", "rb");
    int id, i, n;
    double amount;
    if (f)
    {
        while (fread(&id, sizeof(int), 1, f) == 1 && fread(&amount, sizeof(double), 1, f) == 1)
            for (i = 0; i < employee_count; i++)
                if (employees[i].id == id)
                {
                    personal_wallet[i] = amount;
                    break;
                }
        fclose(f);
    }
    f = fopen("wallet_debits.dat", "rb");
    if (f)
    {
        if (fread(&n, sizeof(int), 1, f) == 1 && n >= 0 && n <= MAX_ORDERS &&
            fread(debits, sizeof(WalletDebit), n, f) == (size_t)n)
            debit_count = n;
        fclose(f);
    }
}
void show_employee_wallet(int i)
{
    printf("Monthly allowance: Rs.%.2f (resets to Rs.3000 monthly)\n", employees[i].wallet);
    printf("Personal wallet: Rs.%.2f (top-ups and loyalty, never expires)\n", personal_wallet[i]);
    printf("Total available: Rs.%.2f\n", employees[i].wallet + personal_wallet[i]);
}

const char *ITEM_FILE = "items.dat", *EMP_FILE = "employees.dat", *STUDENT_FILE = "students.dat", *ORDER_FILE = "orders.dat";

int read_int(const char *prompt)
{
    char buf[100];
    int x;
    char extra;
    while (1)
    {
        printf("%s", prompt);
        if (!fgets(buf, sizeof(buf), stdin))
            exit(0);
        if (sscanf(buf, " %d %c", &x, &extra) == 1)
            return x;
        puts("Please enter a valid number.");
    }
}
double read_money(const char *prompt)
{
    char buf[100];
    double x;
    char extra;
    while (1)
    {
        printf("%s", prompt);
        if (!fgets(buf, sizeof(buf), stdin))
            exit(0);
        if (sscanf(buf, " %lf %c", &x, &extra) == 1 && x >= 0 && x <= 10000000)
            return x;
        puts("Please enter a valid non-negative amount.");
    }
}
void read_text(const char *prompt, char *out, size_t n)
{
    printf("%s", prompt);
    if (!fgets(out, (int)n, stdin))
        exit(0);
    out[strcspn(out, "\n")] = 0;
    if (strchr(out, '\0') == out)
        strcpy(out, "Unknown");
}
void today(char *out)
{
    time_t t = time(NULL);
    strftime(out, 11, "%Y-%m-%d", localtime(&t));
}
int current_month(void)
{
    time_t t = time(NULL);
    struct tm *p = localtime(&t);
    return (p->tm_year + 1900) * 100 + p->tm_mon + 1;
}
void save_data(void)
{
    FILE *f;
    f = fopen(ITEM_FILE, "wb");
    if (f)
    {
        fwrite(&item_count, sizeof(int), 1, f);
        fwrite(items, sizeof(Item), item_count, f);
        fclose(f);
    }
    f = fopen(EMP_FILE, "wb");
    if (f)
    {
        fwrite(&employee_count, sizeof(int), 1, f);
        fwrite(employees, sizeof(Employee), employee_count, f);
        fclose(f);
    }
    f = fopen(STUDENT_FILE, "wb");
    if (f)
    {
        fwrite(&student_count, sizeof(int), 1, f);
        fwrite(students, sizeof(Student), student_count, f);
        fclose(f);
    }
    f = fopen(ORDER_FILE, "wb");
    if (f)
    {
        fwrite(&order_count, sizeof(int), 1, f);
        fwrite(orders, sizeof(Order), order_count, f);
        fclose(f);
    }
    save_wallet_details();
}
int load_array(const char *path, void *arr, int *count, int maximum, size_t size)
{
    FILE *f = fopen(path, "rb");
    int n;
    if (!f)
        return 0;
    if (fread(&n, sizeof(int), 1, f) != 1 || n < 0 || n > maximum || fread(arr, size, n, f) != (size_t)n)
    {
        fclose(f);
        return 0;
    }
    fclose(f);
    *count = n;
    return 1;
}
void init_data(void)
{
    if (!load_array(ITEM_FILE, items, &item_count, MAX_ITEMS, sizeof(Item)))
    {
        Item sample[] = {{101, "Samosa", 15, 8, 50, 1}, {102, "Veg Burger", 60, 35, 30, 1}, {103, "Cold Coffee", 40, 22, 25, 1}, {104, "Sandwich", 50, 28, 35, 1}, {105, "Lunch Thali", 100, 65, 20, 1}};
        item_count = 5;
        memcpy(items, sample, sizeof(sample));
    }
    if (!load_array(EMP_FILE, employees, &employee_count, MAX_EMPLOYEES, sizeof(Employee)))
    {
        Employee e = {201, "Demo Employee", "emp123", WALLET_ALLOWANCE, 0, 1};
        e.wallet_month = current_month();
        employees[0] = e;
        employee_count = 1;
    }
    if (!load_array(STUDENT_FILE, students, &student_count, MAX_STUDENTS, sizeof(Student)))
        student_count = 0;
    if (!load_array(ORDER_FILE, orders, &order_count, MAX_ORDERS, sizeof(Order)))
        order_count = 0;
    save_data();
}
int find_student(int id);
int find_employee(int id);
void refresh_wallet(int i);
/* Reward only once when an order is completed. NGO donations do not earn points. */
int was_rewarded(int order_id)
{
    int i;
    for (i = 0; i < rewarded_count; i++)
        if (rewarded_ids[i] == order_id)
            return 1;
    return 0;
}
void load_rewards(void)
{
    FILE *f = fopen("rewards.dat", "rb");
    int id;
    if (!f)
        return;
    while (rewarded_count < MAX_ORDERS && fread(&id, sizeof(id), 1, f) == 1)
        if (!was_rewarded(id))
            rewarded_ids[rewarded_count++] = id;
    fclose(f);
}
void credit_loyalty(Order *o)
{
    int points, i;
    double food_paid;
    FILE *f;
    if (o->customer_type != 1 && o->customer_type != 2)
        return;
    if (was_rewarded(o->id))
        return;
    food_paid = o->subtotal - o->discount;
    points = (int)((food_paid + 0.000001) / 100.0);
    if (points <= 0)
        return;
    if (o->customer_type == 1)
    {
        i = find_student(o->customer_id);
        if (i < 0)
            return;
        students[i].wallet += points * 10.0;
    }
    else
    {
        i = find_employee(o->customer_id);
        if (i < 0)
            return;
        refresh_wallet(i);
        personal_wallet[i] += points * 10.0;
    }
    rewarded_ids[rewarded_count++] = o->id;
    f = fopen("rewards.dat", "ab");
    if (f)
    {
        fwrite(&o->id, sizeof(int), 1, f);
        fclose(f);
    }
    save_data();
    printf("Loyalty reward: %d point(s) = Rs.%d credited instantly to wallet.\n", points, points * 10);
}
int find_item(int id)
{
    int i;
    for (i = 0; i < item_count; i++)
        if (items[i].id == id && items[i].active)
            return i;
    return -1;
}
int find_employee(int id)
{
    int i;
    for (i = 0; i < employee_count; i++)
        if (employees[i].id == id && employees[i].active)
            return i;
    return -1;
}
int find_student(int id)
{
    int i;
    for (i = 0; i < student_count; i++)
        if (students[i].id == id && students[i].active)
            return i;
    return -1;
}
int find_order(int id)
{
    int i;
    for (i = 0; i < order_count; i++)
        if (orders[i].id == id)
            return i;
    return -1;
}
void menu(void)
{
    int i;
    puts("\nID    FOOD ITEM                      PRICE     STOCK");
    puts("----------------------------------------------------");
    for (i = 0; i < item_count; i++)
        if (items[i].active)
            printf("%-5d %-30s Rs.%-7.2f %d\n", items[i].id, items[i].name, items[i].price, items[i].stock);
}
void inventory(void)
{
    int i;
    puts("\nID    ITEM                     SELL PRICE  COST PRICE  STOCK");
    for (i = 0; i < item_count; i++)
        if (items[i].active)
            printf("%-5d %-24s %10.2f %11.2f %6d%s\n", items[i].id, items[i].name, items[i].price, items[i].cost, items[i].stock, items[i].stock < 5 ? "  LOW STOCK" : "");
}
void add_item(void)
{
    Item *p;
    int id, i;
    if (item_count >= MAX_ITEMS)
    {
        puts("Item storage full.");
        return;
    }
    id = read_int("New item ID: ");
    for (i = 0; i < item_count; i++)
        if (items[i].id == id)
        {
            puts("ID already exists (including deleted items). Use another ID.");
            return;
        }
    p = &items[item_count];
    memset(p, 0, sizeof(*p));
    p->id = id;
    read_text("Item name: ", p->name, sizeof(p->name));
    p->price = read_money("Selling price: Rs.");
    p->cost = read_money("Cost price: Rs.");
    p->stock = read_int("Opening stock: ");
    if (p->stock < 0)
    {
        puts("Invalid stock.");
        return;
    }
    p->active = 1;
    item_count++;
    save_data();
    puts("Item added.");
}
void delete_item(void)
{
    int id = read_int("Item ID to delete: "), i = find_item(id);
    char answer[10];
    if (i < 0)
    {
        puts("Item not found.");
        return;
    }
    printf("Delete %s from menu? (y/n): ", items[i].name);
    fgets(answer, sizeof(answer), stdin);
    if (answer[0] == 'y' || answer[0] == 'Y')
    {
        items[i].active = 0;
        save_data();
        puts("Item removed from menu. Past order history remains.");
    }
}
void update_price(void)
{
    int id = read_int("Item ID: "), i = find_item(id);
    if (i < 0)
    {
        puts("Item not found.");
        return;
    }
    items[i].price = read_money("New selling price: Rs.");
    save_data();
    puts("Price updated.");
}
void update_item_details(void)
{
    int id = read_int("Item ID: "), i = find_item(id);
    if (i < 0)
    {
        puts("Item not found.");
        return;
    }
    read_text("New item name: ", items[i].name, sizeof(items[i].name));
    items[i].price = read_money("New selling price: Rs.");
    items[i].cost = read_money("New cost price: Rs.");
    save_data();
    puts("Item updated.");
}
void update_stock(void)
{
    int id = read_int("Item ID: "), i = find_item(id), amount;
    if (i < 0)
    {
        puts("Item not found.");
        return;
    }
    printf("Current stock: %d\n", items[i].stock);
    amount = read_int("Enter stock to ADD (negative to reduce): ");
    if (items[i].stock + amount < 0)
    {
        puts("Stock cannot become negative.");
        return;
    }
    items[i].stock += amount;
    save_data();
    puts("Stock updated.");
}
void low_stock(void)
{
    int i;
    puts("\nLOW STOCK (below 5 units):");
    for (i = 0; i < item_count; i++)
        if (items[i].active && items[i].stock < 5)
            printf("%d  %s  %d left\n", items[i].id, items[i].name, items[i].stock);
}
void add_employee(void)
{
    int id, i;
    Employee *e;
    if (employee_count >= MAX_EMPLOYEES)
    {
        puts("Employee storage full.");
        return;
    }
    id = read_int("New employee ID: ");
    for (i = 0; i < employee_count; i++)
        if (employees[i].id == id)
        {
            puts("ID already used.");
            return;
        }
    e = &employees[employee_count];
    memset(e, 0, sizeof(*e));
    e->id = id;
    read_text("Employee name: ", e->name, sizeof(e->name));
    read_text("Set password: ", e->password, sizeof(e->password));
    e->wallet = WALLET_ALLOWANCE;
    e->wallet_month = current_month();
    e->active = 1;
    employee_count++;
    save_data();
    puts("Employee added with Rs.3000 monthly wallet.");
}
void remove_employee(void)
{
    int id = read_int("Employee ID to remove: "), i = find_employee(id);
    if (i < 0)
    {
        puts("Employee not found.");
        return;
    }
    employees[i].active = 0;
    save_data();
    puts("Employee login disabled; order records kept.");
}
void show_employees(void)
{
    int i;
    puts("\nID    NAME                       WALLET");
    for (i = 0; i < employee_count; i++)
        if (employees[i].active)
            printf("%-5d %-26s Rs.%.2f\n", employees[i].id, employees[i].name, employees[i].wallet);
}
void refresh_wallet(int i)
{
    int month = current_month();
    if (employees[i].wallet_month != month)
    {
        employees[i].wallet = WALLET_ALLOWANCE;
        employees[i].wallet_month = month;
        save_data();
    }
}
double order_discount_rate(double subtotal)
{
    if (subtotal > 2000)
        return 0.20;
    if (subtotal > 1500)
        return 0.15;
    if (subtotal > 1000)
        return 0.10;
    if (subtotal > 500)
        return 0.05;
    return 0.0;
}
void print_bill(const Order *o)
{
    int i;
    printf("\n========== CANTEEN RECEIPT ==========\nOrder ID: %d  Date: %s\n", o->id, o->date);
    if (o->customer_type == 3)
        puts("Customer: Guest (details not stored)");
    else
        printf("Customer: %s %d\n", o->customer_type == 1 ? "Student" : "Employee", o->customer_id);
    for (i = 0; i < o->count; i++)
        printf("%-22s x%-3d Rs.%.2f\n", o->lines[i].name, o->lines[i].quantity, o->lines[i].price * o->lines[i].quantity);
    printf("-------------------------------------\nSubtotal: Rs.%.2f\nDiscount: Rs.%.2f\nEnvironmental donation: Rs.%.2f\nTOTAL: Rs.%.2f\n", o->subtotal, o->discount, o->donation, o->total);
    printf("Payment: %s\n", o->payment_method == 1 ? "Wallet" : o->payment_method == 2 ? "Cash"
                                                                                       : "UPI (simulated)");
    puts("Donation is recorded for an environmental NGO; transfer is not automated.");
    puts("=====================================");
}
void place_order(int type, int customer_index)
{
    Order o;
    int id, qty, idx, i, method;
    char answer[20];
    double food_total;
    if (order_count >= MAX_ORDERS)
    {
        puts("Order storage full.");
        return;
    }
    memset(&o, 0, sizeof(o));
    o.id = order_count ? orders[order_count - 1].id + 1 : 1001;
    o.customer_type = type;
    o.customer_id = type == 1 ? students[customer_index].id : type == 2 ? employees[customer_index].id
                                                                        : 0;
    today(o.date);
    o.status = 0;
    menu();
    while (o.count < MAX_LINES)
    {
        id = read_int("Enter food ID (0 to finish): ");
        if (id == 0)
            break;
        idx = find_item(id);
        if (idx < 0)
        {
            puts("Invalid item ID.");
            continue;
        }
        qty = read_int("Quantity: ");
        if (qty <= 0)
        {
            puts("Quantity must be positive.");
            continue;
        }
        /* Prevent duplicate lines from exceeding available stock. */
        {
            int already = 0;
            for (i = 0; i < o.count; i++)
                if (o.lines[i].item_id == id)
                    already += o.lines[i].quantity;
            if (qty + already > items[idx].stock)
            {
                puts("Not enough stock.");
                continue;
            }
        }
        o.lines[o.count].item_id = id;
        o.lines[o.count].quantity = qty;
        strcpy(o.lines[o.count].name, items[idx].name);
        o.lines[o.count].price = items[idx].price;
        o.lines[o.count].cost = items[idx].cost;
        o.subtotal += qty * items[idx].price;
        o.count++;
        puts("Added to cart.");
    }
    if (!o.count)
    {
        puts("No items selected.");
        return;
    }
    {
        double tier_discount = o.subtotal * order_discount_rate(o.subtotal);
        double after_tier = o.subtotal - tier_discount;
        double employee_discount = type == 2 ? after_tier * 0.05 : 0;
        o.discount = tier_discount + employee_discount;
        food_total = after_tier - employee_discount;
        printf("Order discount: Rs.%.2f  Employee discount: Rs.%.2f\n", tier_discount, employee_discount);
    }
    printf("\nFood subtotal: Rs.%.2f  Discount: Rs.%.2f\n", o.subtotal, o.discount);
    printf("Donate Rs.5 to an environmental protection NGO? (y/n): ");
    fgets(answer, sizeof(answer), stdin);
    o.donation = (answer[0] == 'y' || answer[0] == 'Y') ? DONATION : 0;
    o.total = food_total + o.donation;
    printf("Total payable: Rs.%.2f\n", o.total);
    if (type == 2)
    {
        refresh_wallet(customer_index);
        show_employee_wallet(customer_index);
    }
    else if (type == 1)
        printf("Student wallet balance: Rs.%.2f\n", students[customer_index].wallet);
    puts(type == 3 ? "Payment: 2. Cash  3. UPI (demo)" : "Payment: 1. Wallet  2. Cash  3. UPI (demo)");
    method = read_int("Select payment method: ");
    if (method < 1 || method > 3 || (type == 3 && method == 1))
    {
        puts("Invalid payment option. Order not placed.");
        return;
    }
    if (method == 1)
    {
        double balance = type == 1 ? students[customer_index].wallet : employees[customer_index].wallet + personal_wallet[customer_index];
        if (balance + 0.00001 < o.total)
        {
            puts("Insufficient wallet balance. Use Cash or UPI instead.");
            return;
        }
    }
    printf("Confirm order? (y/n): ");
    fgets(answer, sizeof(answer), stdin);
    if (answer[0] != 'y' && answer[0] != 'Y')
    {
        puts("Order not placed.");
        return;
    }
    o.payment_method = method;
    o.paid = 1;
    if (method == 1)
    {
        if (type == 1)
            students[customer_index].wallet -= o.total;
        else
        {
            double from_allowance = employees[customer_index].wallet < o.total ? employees[customer_index].wallet : o.total;
            double from_personal = o.total - from_allowance;
            employees[customer_index].wallet -= from_allowance;
            personal_wallet[customer_index] -= from_personal;
            if (debit_count < MAX_ORDERS)
            {
                debits[debit_count].order_id = o.id;
                debits[debit_count].allowance_used = from_allowance;
                debits[debit_count].personal_used = from_personal;
                debit_count++;
            }
        }
    }
    for (i = 0; i < o.count; i++)
    {
        idx = find_item(o.lines[i].item_id);
        if (idx >= 0)
            items[idx].stock -= o.lines[i].quantity;
    }
    orders[order_count++] = o;
    save_data();
    print_bill(&o);
    printf("Your collection token is %d. Keep this Order ID for tracking or cancellation.\n", o.id);
    if (type == 1 || type == 2)
        puts("Loyalty: 1 point per Rs.100 of discounted food spending; each point credits Rs.10 when order is completed.");
}
const char *status_name(int status) { return status == 0 ? "Pending" : status == 1 ? "Preparing"
                                                                   : status == 2   ? "Completed"
                                                                                   : "Cancelled"; }
void order_history(int type, int customer_id)
{
    int i, found = 0;
    puts("\nORDER ID   DATE         TOTAL        STATUS");
    for (i = 0; i < order_count; i++)
        if ((type == 0) || (orders[i].customer_type == type && orders[i].customer_id == customer_id))
        {
            printf("%-10d %-12s Rs.%-10.2f %s\n", orders[i].id, orders[i].date, orders[i].total, status_name(orders[i].status));
            found = 1;
        }
    if (!found)
        puts("No orders found.");
}
/* Guest cancellation uses only the Order ID; no guest identity is stored. */
void cancel_by_id(int id, int type, int customer_id)
{
    int i = find_order(id), j, k;
    char answer[20];
    Order *o;
    if (i < 0)
    {
        puts("Order not found.");
        return;
    }
    o = &orders[i];
    if (o->customer_type != type || (type != 3 && o->customer_id != customer_id))
    {
        puts("This order cannot be cancelled from this account or guest option.");
        return;
    }
    if (o->status != 0)
    {
        printf("Cannot cancel. Current status: %s\n", status_name(o->status));
        return;
    }
    printf("Cancel order %d for Rs.%.2f? (y/n): ", id, o->total);
    if (!fgets(answer, sizeof(answer), stdin))
        return;
    if (answer[0] != 'y' && answer[0] != 'Y')
        return;
    o->status = 3;
    for (j = 0; j < o->count; j++)
        for (k = 0; k < item_count; k++)
            if (items[k].id == o->lines[j].item_id)
            {
                items[k].stock += o->lines[j].quantity;
                break;
            }
    if (o->payment_method == 1)
    {
        if (type == 1)
        {
            int si = find_student(customer_id);
            if (si >= 0)
                students[si].wallet += o->total;
        }
        else if (type == 2)
        {
            int ei = find_employee(customer_id);
            if (ei >= 0)
            {
                refresh_wallet(ei);
                {
                    int d, found = 0;
                    for (d = 0; d < debit_count; d++)
                        if (debits[d].order_id == o->id)
                        {
                            /* If the month changed, expired allowance is not restored. */
                            if (employees[ei].wallet_month == (atoi(o->date) * 100 + atoi(o->date + 5)))
                                employees[ei].wallet += debits[d].allowance_used;
                            personal_wallet[ei] += debits[d].personal_used;
                            found = 1;
                            break;
                        }
                    if (!found)
                        employees[ei].wallet += o->total; /* legacy order */
                }
            }
        }
        puts("Wallet refund processed.");
    }
    else
        puts("Cash/UPI refund marked as due; staff must process it manually.");
    save_data();
    puts("Order cancelled and inventory restored.");
}
void cancel_order(int student_index)
{
    int id = read_int("Order ID to cancel: ");
    cancel_by_id(id, 1, students[student_index].id);
}
void direct_cancel_guest(void)
{
    int id = read_int("Enter guest Order ID to cancel: ");
    cancel_by_id(id, 3, 0);
}
void guest_order(void)
{
    char name[50], phone[25];
    puts("\n=== GUEST ORDER (NO LOGIN) ===");
    read_text("Guest name: ", name, sizeof(name));
    read_text("Phone number: ", phone, sizeof(phone));
    if (strcmp(name, "Unknown") == 0 || strcmp(phone, "Unknown") == 0)
    {
        puts("Name and phone are required.");
        return;
    }
    puts("Guest details are used only during this visit and are not saved.");
    printf("Welcome, %s!\n", name);
    place_order(3, -1);
    /* Name and phone are local variables only, never written to .dat files. */
}
void update_order_status(void)
{
    int id, i, status;
    puts("\n=== UPDATE ORDER STATUS ===");
    order_history(0, 0);
    id = read_int("Enter Order ID: ");
    i = find_order(id);
    if (i < 0)
    {
        puts("Order not found.");
        return;
    }
    printf("Current status: %s\n", status_name(orders[i].status));
    if (orders[i].status == 3)
    {
        puts("Cancelled orders cannot be changed.");
        return;
    }
    if (orders[i].status == 2)
    {
        puts("This order is already completed.");
        return;
    }
    puts("1. Preparing\n2. Completed\n0. Go Back");
    status = read_int("Choose new status: ");
    if (status == 0)
        return;
    if (status != 1 && status != 2)
    {
        puts("Invalid choice.");
        return;
    }
    if (status == orders[i].status)
    {
        puts("Order already has that status.");
        return;
    }
    /* Staff may complete a pending order directly if preparation was not recorded. */
    orders[i].status = status;
    if (status == 2)
        credit_loyalty(&orders[i]);
    save_data();
    printf("Order %d status changed to %s.\n", id, status_name(status));
}
void sales_report(int monthly)
{
    char date[11], prefix[8];
    int i, j, number = 0, cancelled = 0;
    double revenue = 0, cost = 0, donations = 0, discounts = 0, profit;
    today(date);
    memcpy(prefix, date, 7);
    prefix[7] = 0;
    for (i = 0; i < order_count; i++)
    {
        Order *o = &orders[i];
        int match = monthly ? strncmp(o->date, prefix, 7) == 0 : strcmp(o->date, date) == 0;
        if (!match)
            continue;
        if (o->status == 3)
        {
            cancelled++;
            continue;
        }
        number++;
        revenue += o->subtotal - o->discount;
        donations += o->donation;
        discounts += o->discount;
        for (j = 0; j < o->count; j++)
            cost += o->lines[j].cost * o->lines[j].quantity;
    }
    profit = revenue - cost;
    printf("\n========== %s REPORT (%s) ==========\n", monthly ? "MONTHLY" : "DAILY", monthly ? prefix : date);
    printf("Active orders: %d\nCancelled orders: %d\nGross food sales: Rs.%.2f\nEmployee discounts: Rs.%.2f\nNet food sales: Rs.%.2f\nFood cost: Rs.%.2f\n", number, cancelled, revenue + discounts, discounts, revenue, cost);
    printf("Gross %s: Rs.%.2f\n", profit >= 0 ? "profit" : "loss", profit >= 0 ? profit : -profit);
    printf("Environmental donations collected: Rs.%.2f\n", donations);
    puts("Note: This is gross profit/loss before rent, wages, utilities and other expenses.");
    puts("Cash/UPI refunds are assumed completed for cancelled orders in this demo.");
}
/* Demo wallet top-up: no real payment gateway is connected. */
void add_wallet_money(int type, int index)
{
    double amount, *wallet;
    int method;
    if (type == 2)
        refresh_wallet(index);
    wallet = type == 1 ? &students[index].wallet : &personal_wallet[index];
    if (type == 2)
        show_employee_wallet(index);
    else
        printf("Current wallet balance: Rs.%.2f\n", *wallet);
    amount = read_money("Enter amount to add (Rs.): ");
    if (amount <= 0 || amount > 100000)
    {
        puts("Top-up must be between Rs.0.01 and Rs.100000.");
        return;
    }
    if (*wallet + amount > 1000000)
    {
        puts("Wallet limit exceeded (Rs.1000000). ");
        return;
    }
    puts("Top-up method: 1. Cash  2. UPI (simulated)");
    method = read_int("Select method: ");
    if (method != 1 && method != 2)
    {
        puts("Invalid payment method. No money added.");
        return;
    }
    puts("DEMO ONLY: Payment is not actually processed.");
    if (read_int("Confirm simulated payment and wallet credit? (1=Yes, 0=No): ") != 1)
    {
        puts("Top-up cancelled.");
        return;
    }
    *wallet += amount;
    save_data();
    printf("Rs.%.2f added successfully. New balance: Rs.%.2f\n", amount, *wallet);
    if (type == 2)
        puts("Personal top-ups and loyalty rewards carry over every month.");
}
void student_dashboard(int index)
{
    int c, id;
    while (1)
    {
        puts("\n=== STUDENT DASHBOARD ===\n1. View Menu\n2. Place Order\n3. View Order History\n4. Cancel Pending Order\n5. View Order Status / Bill\n6. Check Wallet\n7. Add Money to Wallet\n8. Logout");
        c = read_int("Choice: ");
        switch (c)
        {
        case 1:
            menu();
            break;
        case 2:
            place_order(1, index);
            break;
        case 3:
            order_history(1, students[index].id);
            break;
        case 4:
            cancel_order(index);
            break;
        case 5:
            id = read_int("Order ID: ");
            {
                int k = find_order(id);
                if (k >= 0 && orders[k].customer_type == 1 && orders[k].customer_id == students[index].id)
                {
                    print_bill(&orders[k]);
                    printf("Status: %s\n", status_name(orders[k].status));
                }
                else
                    puts("Order not found.");
            }
            break;
        case 6:
            printf("Wallet balance: Rs.%.2f\n", students[index].wallet);
            break;
        case 7:
            add_wallet_money(1, index);
            break;
        case 8:
            return;
        default:
            puts("Invalid choice.");
        }
    }
}
void employee_dashboard(int index)
{
    int c;
    while (1)
    {
        refresh_wallet(index);
        puts("\n=== EMPLOYEE DASHBOARD ===\n1. View Menu\n2. Order Food (5% Discount)\n3. Order History\n4. Check Monthly Wallet\n5. View Inventory\n6. Update Stock\n7. Update Food Price\n8. Add Food Item\n9. Low Stock Alerts\n10. View All Orders\n11. Update Order Status\n12. Add Money to Wallet\n13. Logout");
        c = read_int("Choice: ");
        switch (c)
        {
        case 1:
            menu();
            break;
        case 2:
            place_order(2, index);
            break;
        case 3:
            order_history(2, employees[index].id);
            break;
        case 4:
            show_employee_wallet(index);
            break;
        case 5:
            inventory();
            break;
        case 6:
            update_stock();
            break;
        case 7:
            update_price();
            break;
        case 8:
            add_item();
            break;
        case 9:
            low_stock();
            break;
        case 10:
            order_history(0, 0);
            break;
        case 11:
            update_order_status();
            break;
        case 12:
            add_wallet_money(2, index);
            break;
        case 13:
            return;
        default:
            puts("Invalid choice.");
        }
    }
}
void admin_dashboard(void)
{
    int c;
    while (1)
    {
        puts("\n=== ADMIN DASHBOARD ===\n1. View Menu\n2. Add Food Item\n3. Delete Food Item\n4. Update Food Details\n5. View Inventory\n6. Add Employee\n7. Remove Employee\n8. View Employees\n9. Daily Sales Report\n10. Monthly Sales Report\n11. Daily Profit / Loss\n12. Monthly Profit / Loss\n13. View All Orders\n14. View Donations (Daily)\n15. View Donations (Monthly)\n16. Update Order Status\n17. Logout");
        c = read_int("Choice: ");
        switch (c)
        {
        case 1:
            menu();
            break;
        case 2:
            add_item();
            break;
        case 3:
            delete_item();
            break;
        case 4:
            update_item_details();
            break;
        case 5:
            inventory();
            break;
        case 6:
            add_employee();
            break;
        case 7:
            remove_employee();
            break;
        case 8:
            show_employees();
            break;
        case 9:
        case 11:
        case 14:
            sales_report(0);
            break;
        case 10:
        case 12:
        case 15:
            sales_report(1);
            break;
        case 13:
            order_history(0, 0);
            break;
        case 16:
            update_order_status();
            break;
        case 17:
            return;
        default:
            puts("Invalid choice.");
        }
    }
}
void student_register(void)
{
    int id, i;
    Student *s;
    if (student_count >= MAX_STUDENTS)
    {
        puts("Student storage full.");
        return;
    }
    id = read_int("Choose student ID: ");
    for (i = 0; i < student_count; i++)
        if (students[i].id == id)
        {
            puts("ID already exists.");
            return;
        }
    s = &students[student_count];
    memset(s, 0, sizeof(*s));
    s->id = id;
    read_text("Name: ", s->name, sizeof(s->name));
    read_text("Password: ", s->password, sizeof(s->password));
    s->wallet = 0;
    s->active = 1;
    student_count++;
    save_data();
    puts("Student registered. Use Cash or UPI (demo) to pay.");
}
void student_login(void)
{
    int id = read_int("Student ID: "), i = find_student(id);
    char pass[30];
    read_text("Password: ", pass, sizeof(pass));
    if (i < 0 || strcmp(students[i].password, pass) != 0)
    {
        puts("Invalid login.");
        return;
    }
    printf("Welcome, %s!\n", students[i].name);
    student_dashboard(i);
}
void employee_login(void)
{
    int id = read_int("Employee ID: "), i = find_employee(id);
    char pass[30];
    read_text("Password: ", pass, sizeof(pass));
    if (i < 0 || strcmp(employees[i].password, pass) != 0)
    {
        puts("Invalid login.");
        return;
    }
    printf("Welcome, %s!\n", employees[i].name);
    employee_dashboard(i);
}
void admin_login(void)
{
    char pass[30];
    read_text("Admin password: ", pass, sizeof(pass));
    if (strcmp(pass, "admin123") == 0)
        admin_dashboard();
    else
        puts("Incorrect password.");
}
int main(void)
{
    int choice;
    init_data();
    load_wallet_details();
    load_rewards();
    while (1)
    {
        puts("\n=====================================\n     SMART CANTEEN BILLING SYSTEM\n=====================================\n1. Student Login\n2. Student Registration\n3. Employee Login\n4. Admin Login\n5. Guest Order (No Login)\n6. Cancel Guest Order by ID\n7. Exit");
        choice = read_int("Enter your choice: ");
        switch (choice)
        {
        case 1:
            student_login();
            break;
        case 2:
            student_register();
            break;
        case 3:
            employee_login();
            break;
        case 4:
            admin_login();
            break;
        case 5:
            guest_order();
            break;
        case 6:
            direct_cancel_guest();
            break;
        case 7:
            save_data();
            puts("Thank you for visiting!");
            return 0;
        default:
            puts("Invalid choice.");
        }
    }
}
