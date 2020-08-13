/*
 * Implement all your JavaScript in this file!
 */
 "use strict";

 var num1 = '';
 var num2 ='';
 var inputs=[];

 var operators=[];
 var clearInput=false;
 var prevOp='';
 var prevOperand='';
 var operatorClicked = false;
 var equalToOpClicked = false;
 var success = false;
 function clearInputFn ()
 {
   if(clearInput === true) {
     console.log('reset input');
     clearInput = false;
     $("input").val("");
   }
 }

function handleNumber()
{
  if(num1===''){
    num1 = $("input").val();
    inputs.push(num1);
  } else if( num2 === '') {
    num2 = $("input").val();
    inputs.push(num2);
  }
  console.log(num1 + " handleNumber " + num2);
}
function handleOperators()
{
  console.log(num1 + operators+num2);
  var answer=0;
  var a = parseInt(num1);
  var b = parseInt(num2);

  switch(prevOp = operators.shift()) {
    case "+":
       answer = a+b;
       break;
    case "-":
       answer = a-b;
       break;
    case "/":
       answer = (a/b).toFixed(2);
       break;
    case "*":
        answer = a*b;
        break;
      }
      //total += answer;
      $("input").val(answer);
      if(operators.length >= 1) {
        // THis operation was activted by another operation<num+num->
        //keep the total
        num1 = answer;num2="";
      }
}
function handleClearButton()
{
  operators = [];
  inputs=[];
  console.log("clear has been clicked")
  $('#output').html($(this).text());
  $("input").val("");
  num1 = ''; num2 ='';
  prevOperand='';
  prevOp = '';
  success = false;
  equalToOpClicked = false;
  operatorClicked=false;
  clearInput = false;
}

function handleEqualsButton()
{
  console.log("equals button")

  //num2 = $("input").val();
  handleNumber();
  console.log(inputs);
  console.log(operators);
  //I called an equalto check for operator
  if(operators.length === 0){
    console.log("there is a number: " + num1 + " but no operators ");
    num1='';
    return false;
  } else if (num1 === '' || num2 === ''){
      console.log(" There is operator " + operators + " but number " + num2 + " is missing");
      return false;
  }
  if(num1 != "" && num2 != "") {
    handleOperators();
    prevOperand = num2;
    num1='';
    num2='';
    operators = [];
  }
  return true;
}

 $(":button").click( function() {
   if( /\d/.test($(this).val())) {
    console.log($(this).val());
    equalToOpClicked = false;
    operatorClicked = false;
    //clear out the numbers and be ready for the next set
    if(clearInput === true) {
      console.log("previosly clicked an operator " + operators)
      clearInput = false;
      $("input").val("");
    }
    var val = $("input").val() + $(this).val();
    $('#output').html($('#output').text() + $(this).text());
    $("input").val(val);//$(display).val(val);
    //console.log($("input").val());
  }
  else {
    // if(clearInput)
    // {
    //   //Looks like I have pressed another operator instead of a number
    //   operators.shift();//keep the most recent operator input
    // }
    console.log(equalToOpClicked);
    if($(this).text() === "=")
    {
      equalToOpClicked = true;
      if(operatorClicked && !success) {
        console.log("Looks like I clicked 2 operators without an operand");
        return;
      }
      if(operatorClicked && success){
        console.log(" operatorClicked and prev operation was a success");
        num2=prevOperand; operators.push(prevOp);
      }
      success = handleEqualsButton();
    } else if($(this).text() === "C"){
        handleClearButton();
      } else if( operators.length > 0 && equalToOpClicked === false && operatorClicked){
        console.log('Looks like I clicked multiple operators ' + operators.shift())
        operators.push($(this).text());
      } else{
        equalToOpClicked = false;
        $('#output').html($('#output').text() + $(this).text());
        inputs.push($(this).text());
        operators.push($(this).text());
        handleNumber();
        //$("input").val("");
        if(num1 != '' && num2 != '')
          handleOperators();
        }
        clearInput = true; operatorClicked = true;
  }
 })
