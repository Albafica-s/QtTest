#include "calculator.h"

#include <QLabel>
#include <QPushButton>
#include <QtTest/QtTest>

class TestCalculator : public QObject
{
    Q_OBJECT

private slots:
    void mouseDecimalCalculation();
    void keyboardMatchesMouse();
    void repeatedDecimalPointIgnored();
    void consecutiveOperatorsReplaceOperator();
    void divisionByZeroShowsErrorAndRecovers();
    void backspaceEditsCurrentOperand();
    void clearResetsCalculator();
    void continuousOperationAfterEquals();
    void digitAfterResultStartsNewCalculation();
    void mixedMouseAndKeyboard();

private:
    static QString displayText(Calculator &calculator)
    {
        return calculator.findChild<QLabel *>(QStringLiteral("displayLabel"))->text();
    }

    static QString expressionText(Calculator &calculator)
    {
        return calculator.findChild<QLabel *>(QStringLiteral("expressionLabel"))->text();
    }

    static void click(Calculator &calculator, const QString &buttonName)
    {
        auto *button = calculator.findChild<QPushButton *>(buttonName);
        QVERIFY2(button, qPrintable(buttonName));
        QTest::mouseClick(button, Qt::LeftButton);
    }

    static void press(Calculator &calculator, int key)
    {
        QTest::keyClick(&calculator, Qt::Key(key));
    }
};

void TestCalculator::mouseDecimalCalculation()
{
    Calculator calculator;
    calculator.show();

    click(calculator, QStringLiteral("btn1"));
    click(calculator, QStringLiteral("btnDot"));
    click(calculator, QStringLiteral("btn5"));
    click(calculator, QStringLiteral("btnPlus"));
    click(calculator, QStringLiteral("btn2"));
    click(calculator, QStringLiteral("btnDot"));
    click(calculator, QStringLiteral("btn5"));
    click(calculator, QStringLiteral("btnEquals"));

    QCOMPARE(displayText(calculator), QStringLiteral("4"));
    QCOMPARE(expressionText(calculator), QStringLiteral("1.5 + 2.5 ="));
}

void TestCalculator::keyboardMatchesMouse()
{
    Calculator calculator;
    calculator.show();

    press(calculator, Qt::Key_1);
    press(calculator, Qt::Key_Period);
    press(calculator, Qt::Key_5);
    press(calculator, Qt::Key_Plus);
    press(calculator, Qt::Key_2);
    press(calculator, Qt::Key_Period);
    press(calculator, Qt::Key_5);
    press(calculator, Qt::Key_Return);

    QCOMPARE(displayText(calculator), QStringLiteral("4"));
    QCOMPARE(expressionText(calculator), QStringLiteral("1.5 + 2.5 ="));
}

void TestCalculator::repeatedDecimalPointIgnored()
{
    Calculator calculator;
    calculator.show();

    click(calculator, QStringLiteral("btn1"));
    click(calculator, QStringLiteral("btnDot"));
    click(calculator, QStringLiteral("btn2"));
    click(calculator, QStringLiteral("btnDot"));
    click(calculator, QStringLiteral("btn3"));

    QCOMPARE(displayText(calculator), QStringLiteral("1.23"));
}

void TestCalculator::consecutiveOperatorsReplaceOperator()
{
    Calculator calculator;
    calculator.show();

    click(calculator, QStringLiteral("btn8"));
    click(calculator, QStringLiteral("btnPlus"));
    click(calculator, QStringLiteral("btnMultiply"));
    click(calculator, QStringLiteral("btn2"));
    click(calculator, QStringLiteral("btnEquals"));

    QCOMPARE(displayText(calculator), QStringLiteral("16"));
    QCOMPARE(expressionText(calculator), QStringLiteral("8 × 2 ="));
}

void TestCalculator::divisionByZeroShowsErrorAndRecovers()
{
    Calculator calculator;
    calculator.show();

    click(calculator, QStringLiteral("btn8"));
    click(calculator, QStringLiteral("btnDivide"));
    click(calculator, QStringLiteral("btn0"));
    click(calculator, QStringLiteral("btnEquals"));

    QCOMPARE(displayText(calculator), QStringLiteral("除数不能为0"));
    QCOMPARE(expressionText(calculator), QStringLiteral("8 ÷ 0 ="));

    // 错误后输入数字即可开始新的计算。
    click(calculator, QStringLiteral("btn3"));
    click(calculator, QStringLiteral("btnPlus"));
    click(calculator, QStringLiteral("btn2"));
    click(calculator, QStringLiteral("btnEquals"));

    QCOMPARE(displayText(calculator), QStringLiteral("5"));
}

void TestCalculator::backspaceEditsCurrentOperand()
{
    Calculator calculator;
    calculator.show();

    click(calculator, QStringLiteral("btn1"));
    click(calculator, QStringLiteral("btn2"));
    click(calculator, QStringLiteral("btn3"));
    click(calculator, QStringLiteral("btnBackspace"));
    QCOMPARE(displayText(calculator), QStringLiteral("12"));

    click(calculator, QStringLiteral("btnBackspace"));
    QCOMPARE(displayText(calculator), QStringLiteral("1"));

    click(calculator, QStringLiteral("btnBackspace"));
    QCOMPARE(displayText(calculator), QStringLiteral("0"));

    // 退格的键盘按键走同一套逻辑。
    press(calculator, Qt::Key_0);
    press(calculator, Qt::Key_Period);
    press(calculator, Qt::Key_5);
    press(calculator, Qt::Key_Backspace);
    QCOMPARE(displayText(calculator), QStringLiteral("0."));
    press(calculator, Qt::Key_Backspace);
    QCOMPARE(displayText(calculator), QStringLiteral("0"));
}

void TestCalculator::clearResetsCalculator()
{
    Calculator calculator;
    calculator.show();

    click(calculator, QStringLiteral("btn2"));
    click(calculator, QStringLiteral("btnPlus"));
    click(calculator, QStringLiteral("btn3"));
    click(calculator, QStringLiteral("btnEquals"));
    click(calculator, QStringLiteral("btnClear"));

    QCOMPARE(displayText(calculator), QStringLiteral("0"));
    QCOMPARE(expressionText(calculator), QString());

    click(calculator, QStringLiteral("btn4"));
    QCOMPARE(displayText(calculator), QStringLiteral("4"));
}

void TestCalculator::continuousOperationAfterEquals()
{
    Calculator calculator;
    calculator.show();

    click(calculator, QStringLiteral("btn2"));
    click(calculator, QStringLiteral("btnPlus"));
    click(calculator, QStringLiteral("btn3"));
    click(calculator, QStringLiteral("btnEquals"));
    QCOMPARE(displayText(calculator), QStringLiteral("5"));

    click(calculator, QStringLiteral("btnPlus"));
    click(calculator, QStringLiteral("btn4"));
    click(calculator, QStringLiteral("btnEquals"));

    QCOMPARE(displayText(calculator), QStringLiteral("9"));
    QCOMPARE(expressionText(calculator), QStringLiteral("5 + 4 ="));
}

void TestCalculator::digitAfterResultStartsNewCalculation()
{
    Calculator calculator;
    calculator.show();

    click(calculator, QStringLiteral("btn2"));
    click(calculator, QStringLiteral("btnPlus"));
    click(calculator, QStringLiteral("btn3"));
    click(calculator, QStringLiteral("btnEquals"));
    click(calculator, QStringLiteral("btn7"));

    QCOMPARE(displayText(calculator), QStringLiteral("7"));
    QCOMPARE(expressionText(calculator), QString());
}

void TestCalculator::mixedMouseAndKeyboard()
{
    Calculator calculator;
    calculator.show();

    click(calculator, QStringLiteral("btn1"));
    click(calculator, QStringLiteral("btnDot"));
    click(calculator, QStringLiteral("btn5"));
    press(calculator, Qt::Key_Plus);
    click(calculator, QStringLiteral("btn2"));
    click(calculator, QStringLiteral("btnDot"));
    click(calculator, QStringLiteral("btn5"));
    press(calculator, Qt::Key_Return);

    QCOMPARE(displayText(calculator), QStringLiteral("4"));
    QCOMPARE(expressionText(calculator), QStringLiteral("1.5 + 2.5 ="));
}

QTEST_MAIN(TestCalculator)
#include "tst_calculator.moc"
