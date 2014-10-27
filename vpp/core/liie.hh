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
    using s::_V_t;
    using s::_V;

    // 1/ extend grammar to handle plain images: A + B -> plus_exp<image2d, image2d>
    //    Not possible. Would cause to many ambiguities when overloading.
    // 2/ iod::exp_map_reduce(exp, map_fun, reduce_fun); -> get_exp_images.
    // 2/ iod::exp_map_ctx(exp, map_fun, reduce_fun); -> images_to_tuple_accessors.
    
    template <int N>
    struct getter
    {};

    template <typename T>
    auto get_exp_images(image2d<T>& img) { return std::forward_as_tuple(img); }
    template <typename T>
    auto get_exp_images(const image2d<T>& img) { return std::forward_as_tuple(img); }

    template <typename T>
    auto get_exp_images(T& terminal) {
      return std::make_tuple();
    }

    template <typename T>
    auto get_exp_images(iod::function_call_exp<_V_t, T>& call) {
      return get_exp_images(std::get<0>(call.args));
    }

    template <typename T, typename U>
    auto get_exp_images(iod::plus_exp<T, U>& node)
    {
      auto lhs = get_exp_images(node.lhs);
      auto rhs = get_exp_images(node.rhs);

      return std::tuple_cat(lhs, rhs);
    }

    struct get_exp_ranges_visitor
    {
      template <typename T>
      auto operator()(image2d<T>& img) const { return std::make_tuple(img); }
      template <typename T>
      auto operator()(const image2d<T>& img) const { return std::make_tuple(img); }
    };

    struct tuple_exp_reduce
    {
      template <typename... T>
      auto operator()(T... t) { return std::tuple_cat(t...); }
    };

    template <typename E>
    auto get_exp_ranges(E& exp)
    {
      return iod::exp_map_reduce(exp, std::tuple<>(),
                                 get_exp_ranges_visitor(),
                                 tuple_exp_reduce());
    }

    // images_to_tuple_accessors;
    //   Transform an ast such as ranges are replaced with getter<N>,
    //   N is the position of the range in the expression numbered from
    //   left to right.
    struct images_to_tuple_accessors_visitor
    {
      template <typename T, int N>
      auto operator()(image2d<T>& node, std::integral_constant<int, N>)
      {
        return make_pair(getter<N>(), std::integral_constant<int, N + 1>());
      }
    };

    template <typename T>
    auto images_to_tuple_accessors(T& node)
    {
      auto res = iod::exp_transform_iterate(node, images_to_tuple_accessors_visitor(),
                                            std::integral_constant<int, 0>());
      return res.first;
    }

    // Evaluate an expression against a context.
    struct evaluate_visitor
    {
      // _V(getter<N>) -> getter<N>
      template <typename T, typename M, typename C>
      inline auto& operator()(iod::function_call_exp<_V_t, T>& read, M eval, C& ctx) const
      {
        return iod::exp_evaluate(std::get<0>(read.args), eval, ctx);
      }

      // getter<N> -> ctx[N]
      template <int N, typename M, typename C>
      inline auto& operator()(const getter<N>& read, M eval, C& ctx) const
      {
        return std::get<N>(ctx);
      }
      
    };

    template <typename E, typename C>
    auto evaluate(E&& exp, C&& ctx)
    {
      return iod::exp_evaluate(exp, evaluate_visitor(), ctx);
    }

    struct run_liie_pixel_wise
    {
      template <typename E, typename... A>
      auto operator()(E& exp, A&&... args) const
      {
        return pixel_wise(args...) | [&exp] (decltype(*std::declval<get_row_iterator_t<A>>())... t)
        {
          return evaluate(exp, std::forward_as_tuple(t...));
        };
      }
    };
    
    // Evaluate image expression such as _V(A) + _V(B).
    template <typename V>
    auto run_liie(iod::Exp<V>&& _exp)
    {
      V& exp = *(V*) &_exp;
      // 1/ Find images in expression.
      auto images = get_exp_ranges(exp);

      // 2/ Transform images in tuple accessors:
      //      _V(I1) + _V(I2) -> _V(getter<0>) + _V(getter<1>)
      auto kernel_exp = images_to_tuple_accessors(exp);
      //void* x = kernel_exp;

      // Pack arguments for run_liie_pixel_wise.
      auto args = std::tuple_cat(std::make_tuple(kernel_exp), images);

      // 3/ Run the expression.
      return iod::apply(args, run_liie_pixel_wise());
    }
  }

  template <typename E>
  auto pixel_wise(iod::Exp<E>&& e)
  {
    return liie::run_liie(std::forward<iod::Exp<E>>(e));
  }

}
