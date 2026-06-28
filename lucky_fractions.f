! Compile the program with the following command line 
! (assuming the name of the file is lucky_fractions.f95).
! gfortran -std=f2018 -O3 -Wextra -pedantic -Wall -o
! lucky_fractions lucky_fractions.f95

! This program must be considered experimental. It is not
! guaranteed to produce complete or correct
! results. The efficiency of the program can be much improved.
! There is no input error checking.

program lucky_fractions
  implicit none
  integer :: base
  integer, dimension(0:2) :: num = 0
  integer, dimension(0:2) :: denom = 0
  integer :: number_of_lucky_fractions = 0
  integer :: n0
  integer :: n1
  integer :: n2
  integer :: d0
  integer :: d1
  integer :: d2
  integer :: num_value
  integer :: denom_value
  integer :: modified_num_value
  integer :: modified_denom_value
  integer :: place_value
  integer, dimension(0:2) :: modified_num
  integer, dimension(0:2) :: modified_denom 
  integer :: digit
  logical :: cancellation_occurred
  double precision :: q1
  double precision :: q2

  write(*, '(A)', advance='no') 'What number base would you like to used (between 3 and 50 inclusive)? '
  read(*,*)base

  do n0 = 1, base - 1
    num(0) = n0
    do n1 = 0, base - 1
      num(1) = n1
      do n2 = 0, base - 1
        num(2) = n2
        do d0 = 1, base - 1
          denom(0) = d0
          do d1 = 0, base - 1
            denom(1) = d1
            do d2 = 0, base - 1
              denom(2) = d2
              
! Calculate the decimal value of the numerator.
              num_value = to_decimal(num, base)

! Calculate the decimal value of the denominator.
              denom_value = to_decimal(denom, base)
 
              if (num_value .ge. denom_value) cycle
                
! Cancel the digits. Cancelled digits are removed by making them negative.
              modified_num = num
              modified_denom = denom
              
              cancellation_occurred = cancel(modified_num, modified_denom)
              q1 = dble(num_value) / denom_value
 
! Calculate the decimal value of the modified numerator.
              modified_num_value = to_decimal_2(modified_num, base)

! Calculate the decimal value of the modified denominator.
              modified_denom_value = to_decimal_2(modified_denom, base)

              if (modified_denom_value .gt. 0 .and. cancellation_occurred) then
                q2 = dble(modified_num_value) / modified_denom_value

                if (abs(q1 - q2) .lt. 0.0000001) then
     number_of_lucky_fractions = number_of_lucky_fractions + 1
   write(*, '(A, I0, A, I0, A, I0, A, I0, A, I0, A, I0, A)') &
             "<", num(0), "> <", num(1), "> <", & 
             num(2), "> / <", denom(0), "> <", &
             denom(1), "> <", denom(2), ">"
                end if
              end if
            end do
          end do
        end do
      end do
    end do
  end do
  
  write(*, '(A, I0, A, I0)') 'The number of base ', base, ' 3-digit lucky fractions is ', number_of_lucky_fractions
  
  contains
  logical function cancel (num, denom)
    implicit none
! Specification part: declare types of arguments and the function return type
    integer, dimension(0:2) :: num
    integer, dimension(0:2) :: denom
    logical :: cancellation_occurred
    cancellation_occurred = .false.
  
    if (denom(0) .ne. 0) then
      if (denom(0) .eq. num(0)) then
        num(0) = -1
        denom(0) = -2
        cancellation_occurred = .true.
      else if (denom(0) .eq. num(1)) then
        num(1) = -1
        denom(0) = -2
        cancellation_occurred = .true.
      else if (denom(0) .eq. num(2)) then
        num(2) = -1
        denom(0) = -2
        cancellation_occurred = .true.
      end if
    end if

    if (denom(1) .ne. 0) then
      if (denom(1) .eq. num(0)) then
        num(0) = -1
        denom(1) = -2
        cancellation_occurred = .true.
      else if (denom(1) .eq. num(1)) then
        num(1) = -1
        denom(1) = -2
        cancellation_occurred = .true.
      else if (denom(1) .eq. num(2)) then
        num(2) = -1
        denom(1) = -2
        cancellation_occurred = .true.
      end if
    end if

    if (denom(2) .ne. 0) then
      if (denom(2) .eq. num(0)) then
        num(0) = -1
        denom(2) = -2
        cancellation_occurred = .true.
      else if (denom(2) .eq. num(1)) then
        num(1) = -1
        denom(2) = -2
        cancellation_occurred = .true.
      else if (denom(2) .eq. num(2) .and. denom(2) .ne. 0) then
        num(2) = -1
        denom(2) = -2
        cancellation_occurred = .true.
      end if
    end if
    cancel = cancellation_occurred
  end function cancel
    
  integer function to_decimal (value, base)
    implicit none
    integer, dimension(0:2) :: value
    integer :: base
    integer :: number_value
    number_value = 0
    do digit = 0, 3 - 1
      place_value = base ** (2 - digit)
      number_value = number_value + place_value * value(digit)
    end do
    to_decimal = number_value
  end function to_decimal
  
  integer function to_decimal_2(value, base)
    implicit none
    integer, dimension(0:2) :: value
    integer :: base
    integer :: number_value    
    integer :: valid_digit
    number_value = 0
    valid_digit = 3
    do digit = 2, 0, -1
      if (value(digit) .lt. 0) cycle
      place_value = base ** (3 - valid_digit)
      number_value = number_value + place_value * value(digit)
      valid_digit = valid_digit - 1
    end do
    to_decimal_2 = number_value
  end function to_decimal_2

end program lucky_fractions
