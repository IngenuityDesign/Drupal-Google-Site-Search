<?php

// Defines the Google site search base url.
define('GSS_BASE_URL', 'https://www.googleapis.com/customsearch/v1?');
/**
 * Class for interaction with Google Site Search.
 */
class GoogleSiteSearch {

  /**
   * The Google API key to use.
   * @var string
   */
  private $key = NULL;

  /**
   * The Google custom search identifier.
   * @var string
   */
  private $cx = NULL;

  /**
   * The language to use in the search.
   * @var string
   */
  private $language = NULL;

  /**
   * The search query.
   * @var string
   */
  private $query = NULL;

  /**
   * Extra parameters to be passed onto Google CSE.
   * @var array
   */
  private $extraParams = NULL;

  /**
   * Page size, number of results per page.
   * @var int
   */
  private $pageSize = 10; //this is max allowed

  /**
   * Pager size, number of pages to show in the pager.
   * @var int
   */
  private $pagerSize = 19;

  /**
   * Current page in the results.
   * @var int
   */
  private $currentPage = NULL;

  /**
   * Total results for a search.
   * @var int
   */
  private $totalResults = 0;

  /**
   * The search results return from Google.
   * @var SimpleXml
   */
  private $searchResultsXml = NULL;
  
  /**
   * The allowed tags in results from Google.
   * @var array
   */
  private $allowedTags = array('a', 'em', 'strong', 'cite', 'blockquote', 'code', 'ul', 'ol', 'li', 'dl', 'dt', 'dd', 'b');

  /**
   * The search categories
   * @var array
   */
  private $categories = array();

  /**
   * Create a new instance of GoogleSiteSearch.
   *
   * @param string
   */
  public function __construct($query, $key = NULL, $cx = NULL, $pageSize = 10, $extraParams = NULL) {
    $this->query = $query;
    $this->SetCx($cx);
    $this->SetKey($key);
    $this->SetPageSize($pageSize);
    $this->extraParams = $extraParams;
  }

  /**
   * Set Google key.
   *
   * @param string $key The Google key.
   * @return void
   */
  public function SetKey($key) {
    $this->key = $key;
  }

  /**
   * Get Google key
   *
   * @return string The Google key.
   */
  public function GetKey() {
    return $this->key;
  }

   /**
    * Set Search Engine id.
    *
    * @param string $cx The Search engine ctx.
    * @return void
    */
   public function SetCx($cx) {
       $this->cx = $cx;
   }

   /**
    * Get search engine ID
    *
    * @return string The search engine ID.
    */
   public function GetCx() {
       return $this->cx;
   }

  /**
   * Set the language.
   *
   * @param str $language The new language.
   * @return void
   */
  public function SetLanguage($language) {
    $this->language = $language;
  }

  /**
   * Set the page size.
   *
   * @param int $pageSize The new page size.
   * @return void
   */
  public function SetPageSize($pageSize) {
    if ($pageSize > 10) $pageSize = 10;
    if ($pageSize < 1) $pageSize = 1;
    $this->pageSize = $pageSize;
  }

  /**
   * Set the pager size.
   *
   * @param int $pagerSize The new pager size.
   * @return void
   */
  public function SetPagerSize($pagerSize) {
    $this->pagerSize = $pagerSize;
  }

  /**
   * Get current page size.
   *
   * @return int The current page size.
   */
  public function GetPageSize() {
    return $this->pageSize;
  }

  /**
   * Get current pager size.
   *
   * @return int The current pager size.
   */
  public function GetPagerSize() {
    return $this->pagerSize;
  }

  /**
   * Get current page in the results.
   *
   * @return int The current page.
   */
  public function GetCurrentPage() {
    return $this->currentPage;
  }

  /**
   * Get total results for current search.
   *
   * @return int Total number of results.
   */
  public function GetTotalResults() {
    return $this->totalResults;
  }

  /**
   * Get total number of pages for search.
   *
   * @return int Total number of pages.
   */
  public function GetTotalPages() {
    return ($this->totalResults / $this->pageSize + 1);
  }

  /**
   * Get the allowed HTML tags.
   *
   * @return array The allowed HTML tags.
   */
  public function GetAllowedTags() {
    return $this->allowedTags;
  }

  /**
   * Get query current search.
   *
   * @return string The current query.
   */
  public function GetQuery() {
    return $this->query;
  }

  /**
   * Get query extra parameters array.
   *
   * @return array The extra parameters to CSE.
   */
  public function GetExtraParameters() {
    return $this->extraParams;
  }

  /**
   * Get search results.
   *
   * @param string $query The search query.
   * @param int $page The page to get results from.
   * @return array The search results.
   * @throws Exception
   */
  public function GetSearchResults($page = 1) {
    // set page
    $this->currentPage = $page;

    // calculate start postition based on page
    $startPos = (($this->currentPage - 1) * $this->pageSize) + 1;

    $params = array(
        'key'   => $this->GetKey(),
        'cx'    => $this->GetCX(),
        'alt'   => 'json',
        'userIp'    => $_SERVER['REMOTE_ADDR'],

        //api specific parameters
        'start' => $startPos,
        'num' => $this->pageSize,

        'q' => $this->query

    );
    
    // Add language parameter if is set.
    if ($this->language) {
      $params['hl'] = $this->language;
    }
     
    // prepare query parameters for URL assembly
    if (count($this->extraParams) > 0) {
      $params = array_merge($params, $this->extraParams);
    }
    else {
      $extraParamsQuery = NULL;
    }

    // get the search base set in the admin, default to google
    $search_base_url = variable_get('gss_base_url', '');
    $search_base_url = !empty($search_base_url) ? $search_base_url : GSS_BASE_URL;

    $API_Url = url($search_base_url, array('query' => $params));
    // fetch results from google
    $url_response = drupal_http_request($API_Url, array(
        'method' => 'GET',
        'max_redirects' => 1
    ));

    if (isset($url_response->error)) {
      return NULL;
    }
    $results = json_decode($url_response->data);

    if ($results !== FALSE) {
      if (!isset($results->items) || count($results->items) < 1) {
        // no results, return NULL
        return NULL;
      }
      else {
        // save total results

        $this->totalResults = intval($results->searchInformation->totalResults);
        // return results
        return array_merge( $this->processPromotions($results->promotions), $this->processReturnedItems( $results->items ) );
      }
    }
    else {
      throw new Exception('Could not load search results from Google.');
    }
  }

  private function processPromotions( $promotions ) {
      $return_promotions = array();

      foreach( $promotions as $promotion ) {
          $aPromotion = array();
          $aPromotion['info'] = $promotion->pagemap->Item[0]->date ? strtotime($item->pagemap->Item[0]->date) : false;
          $aPromotion['title'] = $promotion->title;
          $aPromotion['description'] = $promotion->bodyLines[0]->title;
          if ($promotion->image && $promotion->image->source) $aPromotion['thumbnail_url'] = $promotion->image->source;
          if ($promotion->link) $aPromotion['url'] = $promotion->link;

          $return_promotions[] = $aPromotion;
      }

      return $return_promotions;
  }

  private function processReturnedItems( $items ) {

      $return_items = array();

      foreach( $items as $item ) {
          $anItem = array();
          $anItem['info'] = $item->pagemap->Item[0]->date ? strtotime($item->pagemap->Item[0]->date) : false;
          $anItem['title'] = filter_xss((string) $item->title, $this->GetAllowedTags());
          $anItem['url'] = check_url((string) $item->link);
          $anItem['htmlUrl'] = $item->htmlFormattedUrl;
          $anItem['description'] = filter_xss((string) $item->htmlSnippet, $this->GetAllowedTags());
          if (isset($item->pagemap) && isset($item->pagemap->cse_thumbnail)) {
              $thumb = $item->pagemap->cse_thumbnail;
              $anItem['thumbnail_url'] = $thumb[0]->src;
          }
          $return_items[] = $anItem;

      }
      return $return_items;

  }

  /**
   * Get a pager for the search results.
   *
   * @return string The pager HTML code.
   */
  public function GetPager($link = '?page=') {

    // get total pages
    $totalPages = $this->GetTotalPages();
    $currentPage = $this->GetCurrentPage();
    $pagerSize = $this->GetPagerSize();
    $halfPagerSize = $pagerSize / 2;

    if ($totalPages < 2) {
      return;
    }

    $html = "<div class=\"item-list google-search-pager\"><ul class=\"pager\">\n";

    // Limit the number of pages to 50
    $totalPages = $totalPages > 50 ? 50 : $totalPages;

    if ($currentPage > 1) {
      $link_page = $link . ($currentPage - 1);
      $html .= "\t<li class=\"pager-previous\"><a href=\"{$link_page}\">" . t('‹ previous') . "</a></li>\n";
    }

    // Defines the start and finish of the pager
    $pagerStart = $currentPage - (round($pagerSize / 2) - 1);
    $pagerFinish = $currentPage + (round($pagerSize / 2) - 1);

    // Start values, if $pagerStart is negative
    if ($pagerStart < 1) {
      $pagerStart = 1;
      $pagerFinish = $pagerSize > $totalPages ? $totalPages : $pagerSize;
    }

    // Limit $pagerFinish to the same as $totalPages
    $pagerFinish = $pagerFinish > $totalPages ? $totalPages : $pagerFinish;

    if ($pagerStart > 1) {
      $html .= "\t<li class=\"pager-ellipsis\">...</li>\n";
    }

    for ($i = $pagerStart; $i <= $pagerFinish; $i++) {
      if ($i != $currentPage) {
        $link_page = $link . $i;
        $html .= "\t<li class=\"pager-item\"><a href=\"{$link_page}\">{$i}</a></li>\n";
      }
      else {
        $html .= "\t<li class=\"pager-current\">{$i}</li>\n";
      }
    }

    if ($pagerFinish < $totalPages) {
      $html .= "\t<li class=\"pager-ellipsis\">...</li>\n";
    }

    if ($currentPage < intval($totalPages)) {
      $link_page = $link . ($currentPage + 1);
      $html .= "\t<li class=\"pager-next\"><a href=\"{$link_page}\">" . t('next ›') . "</a></li>\n";
    }

    // close ol
    $html .= "</ul></div>\n";

    // return pager
    return $html;
  }

  /**
   * Get a head to the search results.
   *
   * @return string The search head HTML code.
   */
  public function GetSearchHead() {
    // get total pages
    $totalResults = $this->GetTotalResults();
    $pageSize = $this->GetPageSize();
    $currentPage = $this->GetCurrentPage();
    $show_start = ($pageSize * $currentPage) - $pageSize + 1;
    $show_end = ($pageSize * $currentPage);

    if($show_end > $totalResults) {
      $show_end = $totalResults;
    }

    $html = '<div class="searchhead">';

    if (variable_get('gss_labels', TRUE) == 1) {
      // adding the tabs for the search labels
      $html .= '<span>' . t("Show only results of type:") . '</span><ul>';

      // checking which one is the active tab
      $active_html_first_element = "";
      $active_html_others = "";
      if (strpos($this->query, "more:") === FALSE) { //there is no faceted search
        $active_html_first_element = 'class="active"';   //then the first element is active
        $term_from_query = "";
      }
      else {
        $query_parts = explode(":", $this->query);
        $term_from_query = end($query_parts);  //in this case we store the category to check later
      }

      $this->query = current(explode(" more:", $this->query)); //resume $this->query to its original value (no categories)
      // first item will always be the original query
      $html .= '<li>' . '<a ' . $active_html_first_element . ' href=' . urlencode($this->query) . '>' . t('All results') . '</a>' . '</li>';

      //loop to create all remaining categorized tabs / queries
      if ($totalResults > 0) {
        foreach ($this->categories as $categ) {
          $active_html_others = ""; //do not inherit from previous loop...
          if ($categ['label'] == $term_from_query) {
            $active_html_others = 'class="active"';
          }
          $new_query = urlencode($this->query) . '+more%3A' . $categ['label']; //builds up the path to the tab links
          $html .= '<li>' . '<a ' . $active_html_others . ' href=' . $new_query . '>' . t($categ['anchor_text'], array(), array('context' => 'gss:search-result:filter')) . '</a>' . '</li>';
        }
      }

      $html .= '</ul>';
    }

    if(variable_get('gss_number_of_results', TRUE) == 1 && $totalResults !== 0) {
      $html .= t('Showing @show_start to @show_end of approximately @totalResults', array('@show_start' => $show_start, '@show_end' => $show_end, '@totalResults' => $totalResults));
    }

    $html .= '</div>';

    // return search head
    return $html;
  }





}
