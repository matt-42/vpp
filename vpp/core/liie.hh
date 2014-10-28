#pragma once

#include <iod/sio.hh>
#include <iod/grammar.hh>
#include <iod/foreach.hh>

#include <iod/grammar_utils.hh>
#include <iod/apply.hh>
#include <iod/callable_traits.hh>

namespace vpp
{
  namespace liie
  {
    struct empty_visitor
    {
    };
    
    // Evaluate an expression against a context.
    struct evaluate_visitor
    {
      template <int N, typename M, typename C>
      inline auto& operator()(iod::int_symbol<N>, M eval, C& ctx) const
      {
        return std::get<N - 1>(ctx);
      }
    };

    template <typename E, typename C>
    auto evaluate(E exp, C& ctx)
    {
      return iod::exp_evaluate(exp, evaluate_visitor(), ctx);
    }

    using s::_Sum; using s::_Sum_t;
    using s::_Avg; using s::_Avg_t;
    using s::_Min; using s::_Min_t;
    using s::_Max; using s::_Max_t;
    using s::_Argmax; using s::_Argmax_t;
    using s::_Argmin; using s::_Argmin_t;

    template <typename P>
    using to_pixel_wise_kernel_argument = decltype(*std::declval<get_row_iterator_t<P>>());

    // Evaluate global expressions such as _Sum, _Avg, _Min, _Max, _Argmin ...
    struct evaluate_global_visitor
    {
      // Sum.
      template <typename I, typename... PS>
      inline auto operator()(const iod::function_call_exp<_Sum_t, I>& n, std::tuple<PS...>& ctx) const
      {
        float sum = 0;
        pixel_wise(ctx)(_No_threads) | [&] (to_pixel_wise_kernel_argument<PS>&&... t)
        { auto tp = std::make_tuple(t...); sum += evaluate(std::get<0>(n.args), tp); };
        return sum;
      }

      // Avg.
      template <typename I, typename... PS>
      inline auto operator()(const iod::function_call_exp<_Avg_t, I>& n, std::tuple<PS...>& ctx) const
      {
        float sum = 0;
        int i = 0;
        pixel_wise(ctx)(_No_threads) | [&] (to_pixel_wise_kernel_argument<PS>&&... t)
        { auto tp = std::make_tuple(t...); sum += evaluate(std::get<0>(n.args), tp); i++; };
        return sum / i;
      }

      // Min.
      template <typename I, typename... PS>
      inline auto operator()(const iod::function_call_exp<_Min_t, I>& n, std::tuple<PS...>& ctx) const
      {
        typedef decltype(std::make_tuple(std::declval<to_pixel_wise_kernel_argument<PS>>()...))
          eval_args_tuple;
        typedef decltype(evaluate(std::get<0>(n.args), std::declval<eval_args_tuple&>())) min_type;
        min_type min_value = std::numeric_limits<min_type>::max();

        pixel_wise(ctx)(_No_threads) | [&] (to_pixel_wise_kernel_argument<PS>&&... t)
        {
          auto tp = std::make_tuple(t...);
          min_value = std::min(min_value, evaluate(std::get<0>(n.args), tp));
        };

        return min_value;
      }

      // Max.
      template <typename I, typename... PS>
      inline auto operator()(const iod::function_call_exp<_Max_t, I>& n, std::tuple<PS...>& ctx) const
      {
        typedef decltype(std::make_tuple(std::declval<to_pixel_wise_kernel_argument<PS>>()...))
          eval_args_tuple;
        typedef decltype(evaluate(std::get<0>(n.args), std::declval<eval_args_tuple&>())) min_type;
        min_type max_value = std::numeric_limits<min_type>::min();

        pixel_wise(ctx)(_No_threads) | [&] (to_pixel_wise_kernel_argument<PS>&&... t)
        {
          auto tp = std::make_tuple(t...);
          max_value = std::max(max_value, evaluate(std::get<0>(n.args), tp));
        };

        return max_value;
      }

      // Argmin.
      template <typename I, typename... PS>
      inline auto operator()(const iod::function_call_exp<_Argmin_t, I>& n, std::tuple<PS...>& ctx) const
      {
        typedef decltype(std::make_tuple(std::declval<to_pixel_wise_kernel_argument<PS>>()...))
          eval_args_tuple;
        typedef decltype(evaluate(std::get<0>(n.args), std::declval<eval_args_tuple&>())) min_type;
        min_type min_value = std::numeric_limits<min_type>::max();
        vint2 min_p(0,0);

        auto fr = std::get<0>(ctx);
        box2d domain(fr.first_point_coordinates(), fr.last_point_coordinates());
        auto ctx2 = std::tuple_cat(ctx, std::make_tuple(domain));
        pixel_wise(ctx2)(_No_threads) | [&] (to_pixel_wise_kernel_argument<PS>&&... t, vint2 p)
        {
          auto tp = std::make_tuple(t...);
          auto v = evaluate(std::get<0>(n.args), tp);
          if (v < min_value)
          {
            min_value = v;
            min_p = p;
          }
         };
        return min_p;
      }
      
    };
    
    template <typename E, typename C>
    auto evaluate_global_expressions(E exp, C&& ctx)
    {
      auto e1 = iod::exp_transform(exp, evaluate_global_visitor(), ctx);
      return iod::exp_evaluate(e1, empty_visitor(), ctx);
    }

  }

}
